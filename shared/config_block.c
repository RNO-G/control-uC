#include "shared/config_block.h" 



void default_init_block(config_block_t * block) 
{
  config_block_t dflt = { 
      .boot_cfg = 
      {
           .n_resets_before_reflash = 0, 
           .recovery_priority_list = {1,2,3,4}
      } , 

      .app_cfg = 
      {
        .wanted_state = { .sbc_power= 1 } ,  
        .station_number = 999, 
        .gps_offset = 8
      }
  }; 

  *block = dflt; 
}
