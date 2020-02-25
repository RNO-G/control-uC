#include "shared/config_block.h" 



void default_init_block(config_block_t * block) 
{
  config_block_t dflt = { 
      .boot_cfg = 
      {
           .n_resets_before_reflash = 3, 
           .recovery_priority_list = {1,2,3,4}
      }
  }; 

  *block = dflt; 
}
