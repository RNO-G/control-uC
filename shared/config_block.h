#ifndef _CONFIG_BLOCK_H
#define _CONFIG_BLOCK_H

#include <stdint.h> 

/** Shared configuration block in SPI flash between application and bootloader. 
 *
 *
 *
 */ 


typedef enum 
{
  BOOT_ROM=0, 
  BOOT_FLASH_SLOT_1 = 1, 
  BOOT_FLASH_SLOT_2 = 2, 
  BOOT_FLASH_SLOT_3 = 3, 
  BOOT_FLASH_SLOT_4 = 4, 
  BOOT_BOOTLOADER =10 
} boot_option_t; 



typedef struct bootloader_cfg
{

  /* 
   * The number of resets before
   * the bootloader will attempt a different 
   * boot option, or 0 to disable this functionality. 
   *
   * If this option is used it is important for the application to
   * clear the reset counter 
   *
   */ 
  int n_resets_before_reflash ; 

  // The priority list for attemped recovery from too many resets
  // if n_resets_before_reflash is exceeded, the next item in this list will be flashed
  // 
  boot_option_t recovery_priority_list[4]; 



} bootloader_cfg_t; 



typedef struct application_cfg
{
  uint8_t station_number; 
  uint64_t wanted_state; 
} application_cfg_t; 

typedef struct config_block
{
  bootloader_cfg_t boot_cfg; 
  application_cfg_t app_cfg; 
} config_block_t; 

void default_init_block(config_block_t * block); 


#endif
