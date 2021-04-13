#include "shared/config_block.h" 


#ifndef INITIAL_STATION_NUMBER
#define INITIAL_STATION_NUMBER 999 
#endif 

#define XSTR(X) STR(X) 
#define STR(X) #X 
#pragma message ("Initial Station Number is " XSTR(INITIAL_STATION_NUMBER)) 


void default_init_boot_cfg(bootloader_cfg_t * bc) 
{

  bootloader_cfg_t dflt =  
  {
    .n_resets_before_reflash = 0, 
    .recovery_priority_list = {1,2,3,4}
  };
  *bc = dflt; 
}

void default_init_app_cfg(application_cfg_t * ac) 
{
   application_cfg_t dflt = 
   {
     .wanted_state = RNO_G_NORMAL_MODE, 
     .station_number = INITIAL_STATION_NUMBER, 
     .gps_offset = 18
   }; 

  *ac = dflt; 
}

void default_init_block(config_block_t * block) 
{
  default_init_boot_cfg(&block->boot_cfg); 
  default_init_app_cfg(&block->app_cfg); 
}
