#include "bootloader_driver_init.h" 
#include "shared/shared_memory.h"
#include "shared/config_block.h"
#include "shared/spi_flash.h" 
#include "linker/map.h" 
#include "shared/programmer.h" 
#include "hal_gpio.h"
#include "shared/io.h" 
#include "hpl_reset.h" 
#include <string.h>
#include "shared/printf.h" 

static config_block_t config_block; 


static void __attribute__((naked)) start_app(uint32_t sp, uint32_t start) 
{
  asm("msr msp,r0\n\
       bx r1\n"); 
}

void relenquish() 
{

  get_shared_memory()->bootloader_done = 1; 
  get_shared_memory()->nboots++; 
  get_shared_memory()->nresets++; 


  system_deinit(); 

  uint32_t sp = *((uint32_t*)__rom_start); 
  uint32_t start = *(((uint32_t*)__rom_start)+1);
  start_app(sp,start); 
}



int check_application(int slot)
{

  uint32_t check_buf[2];

  if (slot == 0) 
  {
    flash_read(&FLASH, __rom_start, (uint8_t*) check_buf,8); 
  }
  else
  {
    spi_flash_application_seek(slot,0); 
    spi_flash_application_read(slot, 8, (uint8_t*) check_buf); 
  }

  if ( check_buf[0] == 0xffffffff && check_buf[1] == 0xffffffff)
  {
    return 0; 
  }

  return 1; 
}



int main(void)
{

  system_init();
  io_init(); 


  //read the bootloader config block 
  spi_flash_read_config_block(&config_block); 

  //initialize shared memory
  init_shared_memory(); 
  
  bootloader_cfg_t * cfg = &config_block.boot_cfg;
  shared_memory_t * shm = get_shared_memory();


  shm->booted_from = BOOT_ROM; 

  int must_run_bootloader = 0; 
  int copy_application = 0; 

  int have_application = check_application(0); 

  //are we supposed to boot from not ROM? 
  if (get_shared_memory()->boot_option == BOOT_BOOTLOADER)
  {
    must_run_bootloader = 1; 
  }
  //is the bootloader enable gpio on? 
  else if ( gpio_get_pin_level(GPIO0))//TODO pick correct pin
  {
    must_run_bootloader = 1; 
  }

  //did we tell ourselves to copy the application ? 
  else if (shm->boot_option > 0 && shm->boot_option <= 4) 
  {
    //check if there is actually an applicatio there! 
    if (check_application(shm->boot_option))
    {
      copy_application = shm->boot_option; 
    }
  }
  //otherwise check if we are in a bad state and need to try to recover
  // This is either because we've reflashed too many times our application is gone. 
  // In this case, we loop through the priority list trying to find a valid application and copy it. 
  else if ((cfg->n_resets_before_reflash && shm->nresets > cfg->n_resets_before_reflash) || !have_application)
  {
    int ntries = 0; 
    do 
    {
      boot_option_t boot_option= cfg->recovery_priority_list[shm->boot_list_index]; 
      shm->boot_list_index = shm->boot_list_index+1 % 4; 

      //check if this is one of the spi flash slots and  that an application exists there
      if (boot_option >0 && boot_option <= 4 && check_application(boot_option))
      {
        copy_application = boot_option; 
        break; 
      }
    } while(ntries++ < 4);
  }

  if (copy_application) 
  {
    
    programmer_copy_application_to_flash(copy_application); 
    shm->nresets = 0; 
    shm->booted_from = copy_application; 
    have_application = 1; 
  }

  if(!have_application)
  {
    must_run_bootloader = 1; 
  }

  //otherwise, check if we should run the bootloader
  if (must_run_bootloader) 
  {
    sbc_uart_put("BOOTLOADER!\r\n"); 

#define bufsize 64
    char buf[bufsize+1] = {0}; 
    int offset = 0;
    int programmer_entered = 0; 
    while(1) 
    {
      if (programmer_entered)
      {
        if (!programmer_process()) 
        {
          programmer_entered = 0;
        }
        continue; 
      }

      offset+=sbc_uart_read(bufsize-offset, (uint8_t*) buf+offset); 
      //check for line returns
      char * lr = strchr(buf,'\n'); 
      while (lr) 
      {
        *lr = 0; 
        if (programmer_check_command(buf))
        {
          programmer_enter(buf,SBC_UART_DESC);
          programmer_entered = 1;
          break;
        }
        else if (strcmp(buf,"#RESET"))
        {
          _reset_mcu();
        }
        else if (strcmp(buf,"#EXIT"))
        {
          break;
        }
        else
        {
          //otherwise, move to front of buffer
         int  len = lr-buf;
          memmove(lr+1, buf, len);
          offset-=len; 
        }
        lr = strchr(buf,'\n'); 
      }
      if (offset > bufsize/2)
      {
        //move back to front if over half
        memmove(buf+bufsize/2, buf, bufsize/2); 
        memset(buf+bufsize/2,0,bufsize/2); 
        offset-=bufsize/2; 
      }
    }
  }

  relenquish(); 
}
