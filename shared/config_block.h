#ifndef _CONFIG_BLOCK_H
#define _CONFIG_BLOCK_H

#include <stdint.h> 
#include "rno-g-control.h" 

#define CONFIG_BLOCK_BOOT_VERSION 1
#define CONFIG_BLOCK_APP_VERSION 2

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
  uint16_t station_number; 
  int8_t gps_offset; 
  rno_g_mode_t wanted_state; 
  sbc_boot_mode_t sbc_boot_mode; 
  int report_interval; 
  int report_interval_low_power_mode; 
  int lte_stats_interval; 
  int lora_stats_interval; 
} application_cfg_t; 

typedef struct config_block
{
  bootloader_cfg_t boot_cfg; 
  application_cfg_t app_cfg; 
} config_block_t; 

void default_init_block(config_block_t * block); 
void verify_app_cfg(application_cfg_t *ac); 
void default_init_boot_cfg(bootloader_cfg_t * bc); 
void default_init_app_cfg(application_cfg_t * ac); 

#endif
