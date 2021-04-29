#include "shared/shared_memory.h" 
#include "hpl_reset.h" 


#define MAGIC_START 0xbada110c 
#define MAGIC_END   0x70c0fefe 


void init_shared_memory() 
{
//can only be initialized by bootloader
#ifdef _BOOTLOADER_
  shared_memory_t * shm = get_shared_memory(); 
  //check magic or POR 
  if ( _get_reset_reason() == RESET_REASON_POR || 
      shm->magic_start!=MAGIC_START || 
      shm->magic_end  !=MAGIC_END ) 
  {
    // detected first_boot !

    shm->nresets = 0; 
    shm->nboots = 1; 
    shm->ncrash = 0; 
    shm->crash_reason = CRASH_UNSET; 
    shm->boot_option = BOOT_ROM; 
    shm->crash_data[0] = 0; //null terminated 
    shm->magic_start = MAGIC_START;
    shm->magic_end = MAGIC_END;
    shm->boot_list_index = 0; 
    shm->booted_from = BOOT_ROM; 
  }
  else
  {
    //just increment boot count 
    shm->nboots++; 
    return;
  }
#endif
}


