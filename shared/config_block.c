#include "shared/config_block.h" 


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
     .wanted_state = { .sbc_power= 1 } ,  
     .station_number = 999, 
     .gps_offset = 18
   }; 

  *ac = dflt; 
}

void default_init_block(config_block_t * block) 
{
  default_init_boot_cfg(&block->boot_cfg); 
  default_init_app_cfg(&block->app_cfg); 
}
