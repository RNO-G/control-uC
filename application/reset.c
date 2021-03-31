#include "application/reset.h"
#include "shared/shared_memory.h" 
#include "hpl_reset.h"


int reset(boot_option_t opt) 
{
  get_shared_memory()->boot_option = opt;
  _reset_mcu(); 
}





