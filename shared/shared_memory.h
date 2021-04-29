#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H

#include "shared/config_block.h" 
#include "linker/map.h" 

/** Shared memory between the application and the bootloader,
 * mostly for crash recovery/ diagonstics */ 

typedef struct shared_memory 
{
  int magic_start; //used to detect if we've retained values

  int nresets;  //number of "too fast" resets 
  int nboots;  //number of boots since ram was last cleared... 

  enum 
  {
    CRASH_UNSET = 0, 
    CRASH_WATCHDOG_EXPIRED = 1, //set by watchdog interrupt 
    CRASH_HARDFAULT = 2
  } crash_reason; //surely we can think of more things!

  // If application sets this to non-ROM then reset
  //  the application with that index is copied into flash 
  //  this will then be set back to BOOT_ROM afterwards, but
  //  the booted_from will tell us what we booted from. 
  boot_option_t boot_option ; 
  boot_option_t booted_from; 

  // index within boot priority list (defined by config) 
  int boot_list_index ; 

  // bootloader should set this to true at end 
  int bootloader_done;  

  // might be filled by an interrupt handler if we're lucky . Null terminated. 
  char crash_data[192]; 
  int magic_end; //used to detect if we've retained values

} shared_memory_t; 

// Check if shared memory has been initialized and if not initialize it. 
void init_shared_memory(); 

static inline shared_memory_t * get_shared_memory()  { return (shared_memory_t*)  &__shared_start__; } 


#endif 
