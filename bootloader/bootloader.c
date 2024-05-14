#include "shared/driver_init.h"
#include "shared/shared_memory.h"
#include "shared/config_block.h"
#include "shared/spi_flash.h"
#include "linker/map.h"
#include "shared/programmer.h"
#include "hal_gpio.h"
#include "hpl_reset.h"
#include "shared/printf.h"
#include "shared/io.h"

void HardFault_Handler()
{
  get_shared_memory()->crash_reason = CRASH_HARDFAULT;
  get_shared_memory()->ncrash++;
  _reset_mcu();
}



static void __attribute__((naked)) start_app(uint32_t sp, uint32_t start)
{
  asm("msr msp,r0\n\
       bx r1\n");

  (void) sp;
  (void) start;
}


void relenquish()
{

  get_shared_memory()->bootloader_done = 1;
  get_shared_memory()->nboots++;
  get_shared_memory()->nresets++;

  system_deinit();

  uint32_t sp = *((uint32_t*)&__rom_start__);
  uint32_t start = *(((uint32_t*)&__rom_start__)+1);
  start_app(sp,start);
}



int check_application(int slot)
{

  uint32_t check_buf[2];

  if (slot == 0)
  {
    flash_read(&FLASH, (int) &__rom_start__, (uint8_t*) check_buf,8);
  }
  else
  {
    spi_flash_wakeup();
    spi_flash_application_seek(slot,0);
    spi_flash_application_read(slot, 8, (uint8_t*) check_buf);
  }

  if ( check_buf[0] == 0xffffffff && check_buf[1] == 0xffffffff)
  {
    return 0;
  }

  return 1;
}


ASYNC_TOKENIZED_BUFFER(512, sbc,"\r\n", SBC_UART_DESC);


int main(void)
{

  system_init();

  spi_flash_init();

  //read the bootloader config block
  config_block_t * cfgblock = config_block();

  //initialize shared memory
  init_shared_memory();

  bootloader_cfg_t * cfg = &cfgblock->boot_cfg;
  volatile shared_memory_t * shm = get_shared_memory();


  shm->booted_from = BOOT_ROM;

  volatile int must_run_bootloader = 0;
  int copy_application = 0;

  volatile int have_application = check_application(0);
  //are we supposed to boot from not ROM?
  if (get_shared_memory()->boot_option == BOOT_BOOTLOADER)
  {
    must_run_bootloader = 1;
    shm->boot_option = 0;
  }
  //is the bootloader enable gpio on?
//  else if ( gpio_get_pin_level(GPIO0))//TODO pick correct pin
//  {
//    must_run_bootloader = 1;
//  }

  //did we tell ourselves to copy the application ?
  else if (shm->boot_option > 0 && shm->boot_option <= 4)
  {
    //check if there is actually an applicatio there!
    if (check_application(shm->boot_option))
    {
      copy_application = shm->boot_option;
    }
    shm->boot_option = 0; //reset to ROM after
  }
  //otherwise check if we are in a bad state and need to try to recover
  // This is either because we've reflashed too many times or our application is gone.
  // In this case, we loop through the priority list trying to find a valid application and copy it.
  else if ((cfg->n_resets_before_reflash >0  && shm->nresets > cfg->n_resets_before_reflash) || !have_application)
  {
    int ntries = 0;
    do
    {
      rno_g_boot_option_t boot_option= cfg->recovery_priority_list[shm->boot_list_index];
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
    sbc_io_init();
    if (!have_application)
    {
      printf("#BOOTLOADER: NO VALID APPLICATION IN FLASH!\n");
    }

    printf("#INFO: COPYING FROM SLOT %d to FLASH\r\n", copy_application);


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
    sbc_io_init();
    sbc_uart_put("#IN BOOTLOADER!\r\n");

    while(1)
    {
      programmer_process();


      while (async_tokenized_buffer_ready(&sbc) )
      {
        if (programmer_check_command(sbc.buf))
        {
          programmer_cmd(sbc.buf,sbc.len);
        }
        else if (prefix_matches(sbc.buf,"#SYS-RESET"))
        {
          int opt = 0;
          const char * nxt=0;
          if (parse_int(sbc.buf + sizeof("#SYS-RESET"), &nxt, &opt)) opt=0 ;
          printf("#SYS-RESET(%d)!!\r\n", opt);
          shm->boot_option = opt;
          _reset_mcu();
        }
        else if (!strcmp(sbc.buf,"#EXIT"))
        {
          if (!have_application)
          {
            printf("#ERROR: CAN'T EXIT WITHOUT APPLICATION!\\n");
          }
          else
          {
            printf("#EXIT\n");
            break;
          }
        }
        else if (!strcmp(sbc.buf,"#AM-I-BOOTLOADER"))
        {
          printf("#AM-I-BOOTLOADER: 1\r\n");
        }

        async_tokenized_buffer_discard(&sbc);
      }
      delay_ms(20);
    }
  }

  //Make sure each slot has an application (otherwise recovery depends on someone thinking to do it)
  // if no application in first slot, write one (this way we always have a backup)
  if (have_application)
  {
    sbc_io_init();
    for (int islot = 1; islot <= 4; islot++)
    {
      if (!check_application(islot))
      {
        printf("#INFO: SLOT %d is empty, copying current image\r\n", islot);
        programmer_copy_flash_to_application(islot);
      }
    }
  }

  relenquish();
  return 0;
}
