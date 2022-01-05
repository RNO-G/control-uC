#include "application/reset.h"
#include "shared/shared_memory.h" 
#include "hpl_reset.h"


void reset(rno_g_boot_option_t opt) 
{
  get_shared_memory()->boot_option = opt;
  _reset_mcu(); 
}





