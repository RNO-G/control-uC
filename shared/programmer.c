#include "shared/programmer.h" 
#include "shared/spi_flash.h" 
#include "hal_flash.h" 
#include <stdlib.h>
#include <string.h>
#include "driver_init.h"
#include "linker/map.h" 


#include "shared/io.h" 
#include "shared/printf.h" 



enum commands
{
  PGM_NO_CMD = 0,
  PGM_WRITE_CMD=1,
  PGM_READ_CMD=2,
  PGM_ERASE_CMD =3
}; 


static int which_command(const char * cmd, int * slot) 
{
  if (cmd [0] != '#') return PGM_NO_CMD; 

  int ret = 0; 
  if ( !memcmp(cmd+1, "PRG@", 4)) ret = PGM_WRITE_CMD; 
  else if ( !memcmp(cmd+1, "DEL@", 4)) ret = PGM_ERASE_CMD; 
  else if ( !memcmp(cmd+1, "REA@", 4)) ret = PGM_READ_CMD; 

  if (ret!=0 && slot) 
  {
    *slot = cmd[5] - '0'; 
  }
  return ret; 
}


int programmer_check_command(const char * cmd) 
{
  return !!which_command(cmd,0); 
}

static enum
{
  PROGRAM_IDLE, 
  PROGRAM_WRITE_INIT, 
  PROGRAM_WRITE_CONFIRM, 
  PROGRAM_WRITE_NBLOCKS, 
  PROGRAM_WRITE_WAITING,
  PROGRAM_WRITE_WRITING,
  PROGRAM_READ_INIT, 
  PROGRAM_READ_NBLOCKS, 
  PROGRAM_READ_READING,
  PROGRAM_ERASE_INIT, 
  PROGRAM_ERASE_CONFIRM, 
  PROGRAM_ERASE_NBLOCKS,
  PROGRAM_ERASE_ERASING, 
  PROGRAM_ERROR,
  PROGRAM_DONE
} program_state = PROGRAM_IDLE; 


static int slot; 

// The byte or block counter 
static int i; 

// The byte or block number
static int N; 


static int fd; 
static uint8_t programmer_buf[260]; 
static async_tokenized_buffer_t b = { .buf=programmer_buf, .capacity = sizeof(programmer_buf), .token = "\r\n"}; 

static volatile enum 
{
  FLASH_READY, 
  FLASH_BUSY, 
  FLASH_ERROR
}flash_status ; 


static void flash_ready_callback(struct flash_descriptor * const d) 
{
  (void) d;
  flash_status = FLASH_READY; 
}

static void flash_error_callback(struct flash_descriptor * const d) 
{
  (void) d; 
  flash_status = FLASH_ERROR; 
}


int programmer_enter(const char * cmd, int dev) 
{
  if (program_state) return 1; 
  int check_slot; 
  int which = which_command(cmd,&check_slot); 

#ifndef _BOOTLOADER_
  if (!check_slot)
  {
    d_put(fd,"#ERR\r\n"); 
  }
#endif

  if (!which || check_slot < 0 || check_slot > 4)
  {
    d_put(fd,"#ERR\r\n"); 
    return 1; 
  }
  
  i = 0; 
  fd = dev; 
  b.desc = fd; 


  slot = check_slot; 

  if (slot == 0) 
  {
    flash_register_callback(&FLASH, FLASH_CB_READY, flash_ready_callback); 
    flash_register_callback(&FLASH, FLASH_CB_ERROR, flash_error_callback); 
  }
  else
  {
    spi_flash_wakeup(); 
  }



  switch (which) 
  {
    case PGM_WRITE_CMD: 
      program_state = PROGRAM_WRITE_INIT; 
      break; 
    case PGM_READ_CMD: 
      program_state = PROGRAM_READ_INIT;
      break;
    case PGM_ERASE_CMD:
      program_state = PROGRAM_ERASE_INIT; 
      break; 
    default: 
      program_state = PROGRAM_IDLE; 
  }

  return 0; 
}



static void write_confirm() 
{
  dprintf(fd,"?CONFIRM@%d\r\n",slot); 
}



static int check_confirm_slot(int success_state) 
{
  if (!async_tokenized_buffer_ready(&b)) return 0; 
  if (b.len == 1 && b.buf[0] == slot + '0' )
  {
      program_state = success_state; 
      async_tokenized_buffer_discard(&b); 
      return 1; 
  }
  program_state = PROGRAM_ERROR; 
  return 0; 
}


static int read_nblocks() 
{

  int total = 0; 
  int mult = 1; 
  int buf_len = b.len; 
  for (int idig = 0; idig < buf_len; idig++) 
  {
    char c = b.buf[buf_len-1-idig]; 
    if (c < '0' || c > '9') return 0; 
    total += mult*(c-'0'); 
    mult*=10; 
  }

  async_tokenized_buffer_discard(&b); 

  N = total; 
  return total; 
}



static int accumulate_flash_buffer() 
{
  if  (b.len < 256) 
  {
    b.len+= d_read(fd, 256-b.len, (uint8_t*) b.buf+ b.len); 
  }

  if (b.len >= 256) return 1; 
  return 0; 
}


static void process_write() 
{


  static int transfer_was_started = 0; 
  static int sent_query = -1; 

  //Check if write operation is still in progress
  if ((slot == 0 && flash_status == FLASH_BUSY) || spi_flash_busy() ) 
  {
    return; 
  }
  if (flash_status == FLASH_ERROR) //TODO: can check spi flash status for error
  {
    program_state = PROGRAM_ERROR; 
  }

  

  //this means we are done with the transfer, so we can ask for a new buffer
  if (transfer_was_started)
  {
    transfer_was_started = 0; 
    i++; //move on to the next block

    if (i == N) 
    {
      dprintf(fd,"#DONE_PROG@%d:%d\r\n", slot,N); 
      program_state = PROGRAM_DONE; 
      return; 
    }
    b.len = 0; 
  }

  if (sent_query < i)
  {
    dprintf(fd,"BLOCK_%03d\r\n",i); 
    sent_query = i; 
  }

  //accumulate more bytes, if we have 256, then let's pass to flash
  if (accumulate_flash_buffer())
  {

    //otherwise, let's write 256 bytes
    if (slot==0) 
    {
      flash_append(&FLASH, 8*1024 + 256*i, (uint8_t*) b.buf,256); 
      flash_status = FLASH_BUSY; 
    }
    else
    {
      if (i == 0) spi_flash_application_seek(slot,0); 
      spi_flash_application_write(slot,256, (uint8_t*) b.buf); 
    }

    transfer_was_started = 1;
  }
  return;  

}

//read, 256 bytes at a time I guess? 
static void process_read() 
{
  uint8_t flash_buffer[256]; 
  int nb = N-i >= 256 ? 256 : N-i; 
  if (slot == 0 ) 
  {
    flash_read(&FLASH,8*1024+i,  flash_buffer, nb); 
    i+=nb;
  }
  else 
  {
    if (i == 0) 
    {
      spi_flash_application_seek(slot,0); 
    }
    i+=spi_flash_application_read(slot,nb,  flash_buffer); 
  }

  d_write(fd, nb, flash_buffer); 

  if ( i >=N) 
  {
    dprintf(fd,"#DONE_READ@%d:%d\r\n", slot,N);
    program_state = PROGRAM_DONE; 
  }
}

static void erase_done() 
{

  dprintf(fd,"#DONE_ERASE@%d:%d",slot,N);
  program_state = PROGRAM_DONE; 
}

static void process_erase() 
{

  if (slot == 0) 
  {
    static int erase_started = 0; 

    if (flash_status == FLASH_BUSY) return; 
    if (flash_status == FLASH_ERROR) 
    {
      program_state = PROGRAM_ERROR; 
    }

    if (erase_started) 
    {
      flash_status = FLASH_READY; 
      erase_done(); 
      erase_started = 0; 
      return; 
    }

    flash_erase(&FLASH, 8*1024, N); 
    erase_started = 1; 
  }
  else
  {
    int ret = spi_flash_application_erase_async(slot, N);

    if (ret == 0)
    {
      erase_done(); 
    }
    else if (ret < 0)
    {
      program_state = PROGRAM_ERROR; 
    }
  }
}

int check_nblocks(int success_state)
{
  if (!async_tokenized_buffer_ready(&b)) return 0; 

  if (!read_nblocks())
  {
     i = 0; 
     b.token_matched =0; 
     b.len = 0; 
     program_state = success_state; 
     return 0; 
  }
  else 
  {
     program_state = PROGRAM_ERROR;
     return 1;
  }
 

}


int programmer_process() 
{
  switch (program_state)
  {
    case PROGRAM_IDLE: 
    case PROGRAM_DONE: 
      if (slot) spi_flash_deep_sleep();  //put the flash back to sleep 
      program_state = PROGRAM_IDLE; 
      return 0; 
    case PROGRAM_ERROR: 
      d_put(fd,"#ERR\n"); 
      program_state = PROGRAM_IDLE; 
      return -1;
    case PROGRAM_WRITE_INIT: 
      write_confirm(); 
      program_state = PROGRAM_WRITE_CONFIRM; 
      break; 
    case PROGRAM_WRITE_CONFIRM:  
      if (check_confirm_slot(PROGRAM_WRITE_NBLOCKS))
      {
        (d_put(fd,"?N_256B_BLOCKS\n")); 
      }
      else break;
    case PROGRAM_WRITE_NBLOCKS: 
      check_nblocks(PROGRAM_WRITE_WRITING);
      break; 
   case PROGRAM_WRITE_WRITING:
      process_write() ;
      break; 
    case PROGRAM_READ_INIT: 
      d_put(fd,"?NBYTES"); 
      program_state = PROGRAM_READ_NBLOCKS; 
      break; 
    case PROGRAM_READ_NBLOCKS: 
      if (check_nblocks(PROGRAM_READ_READING)) break; 
    case PROGRAM_READ_READING: 
       process_read(); 
       break; 
    case PROGRAM_ERASE_INIT: 
      write_confirm(); 
      program_state = PROGRAM_ERASE_CONFIRM; 
      break;
    case PROGRAM_ERASE_CONFIRM: 
      if(check_confirm_slot(PROGRAM_ERASE_NBLOCKS))
      {
        d_put(fd,"?N_4KB_BLOCKS\n"); 
      }
      else break; 
    case PROGRAM_ERASE_NBLOCKS: 
      if (check_nblocks(PROGRAM_ERASE_ERASING)) break; 
     case PROGRAM_ERASE_ERASING: 
        process_erase(); 
        break; 
    default: 
      break; 
  }

  return 1; 
}


int programmer_copy_flash_to_application(int slot) 
{

  static int copy_started = 0; 

  if (program_state != PROGRAM_IDLE) 
  {
    //we can't have another programming task going on
    return -1; 
  }

  uint8_t* offset = (uint8_t*) &__rom_start__; 
  if (!copy_started) 
  {
    i = 0;
    N = (int) &__rom_size__; 
    spi_flash_wakeup();
    spi_flash_application_seek(slot,0); 
    copy_started = 1; 
  }

  uint8_t buf[256]; 

  //try doing it all in one go for now? split it up if we need to. 
  while (i < N) 
  {
    int howmany = i + 256 < N ? 256 : N -i; 
    flash_read(&FLASH, (uint32_t) offset+i, buf, howmany); 
    spi_flash_application_write(slot,howmany, buf); 
    i+=howmany; 
  }
  
  copy_started = 0; 
  spi_flash_deep_sleep(); 
  return 0; 
}


int programmer_copy_application_to_flash(int slot) 
{
#ifndef _BOOTLOADER_ 
  (void) slot; 
  return -1; 
#else
  static int copy_started = 0; 

  if (program_state != PROGRAM_IDLE) 
  {
    //we can't have another programmiing task going on
    return -1; 
  }

  if (!copy_started) 
  {
    flash_register_callback(&FLASH, FLASH_CB_READY, flash_ready_callback); 
    flash_register_callback(&FLASH, FLASH_CB_ERROR, flash_error_callback); 
    i = 0;
    N = (int) &__rom_size__; 
    spi_flash_application_seek(slot,0); 
    spi_flash_wakeup();
    copy_started = 1; 
  }

  //we don't know how long the application is, so we just copy everything in the block
  uint8_t flash_buffer[256]; 

  while (i < N) 
  {
    spi_flash_application_read(slot,256, flash_buffer); 
    if (flash_status == FLASH_BUSY) return 1; 
    if (flash_status == FLASH_ERROR) 
    {
      copy_started = 0; //restart the process on the next try 
      return -1; 
    }
    flash_write(&FLASH, &__rom_start__+i,flash_buffer,256); 
    flash_status = FLASH_BUSY; 
  }
  
  copy_started = 0; 
  spi_flash_deep_sleep(); 
  return 0; 
#endif 
}


