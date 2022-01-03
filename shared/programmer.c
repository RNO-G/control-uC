#include "shared/programmer.h" 
#include "shared/spi_flash.h" 
#include "hal_flash.h" 
#include <stdlib.h>
#include <string.h>
#include "driver_init.h"
#include "linker/map.h" 
#include "shared/base64.h" 
#include "shared/io.h" 
#include "shared/printf.h" 



#ifdef _BOOTLOADER_
#define MIN_WRITE_SLOT 0 
#else
#define MIN_WRITE_SLOT 1 
#endif

int programmer_check_command(const char * cmd) 
{
  return prefix_matches(cmd,"#PROG"); 
}


static int spi_flash_awake =0; 
#ifdef _BOOTLOADER_
static int last_slot =-1; 
#endif

static void wakeup_spi_flash()
{
  if (!spi_flash_awake) spi_flash_wakeup(); 
  spi_flash_awake = 1; 
}

static void put_down_spi_flash() 
{

    spi_flash_deep_sleep(); 
    spi_flash_awake = 0; 
}

static void maybe_put_down_spi_flash()
{
  if (!spi_flash_awake) return; 
  if (spi_flash_awake ++> 100) 
  {
    put_down_spi_flash(); 
  }
}


static enum
{
  PROGRAM_IDLE, 
  PROGRAM_ERROR,
  PROGRAM_COPYING_TO_FLASH,
  PROGRAM_DONE
} program_state = PROGRAM_IDLE; 


static volatile enum 
{
  FLASH_READY, 
  FLASH_BUSY, 
  FLASH_ERROR
}flash_status ; 








int programmer_cmd(char * in, int in_len) 
{


      if (prefix_matches(in,"#PROG-COPY-FLASH-TO-SLOT"))
      {
        int slot =-1; 
        const char * nxt = 0; 
        if (!parse_int(in + sizeof("#PROG-COPY-FLASH-TO-SLOT"), &nxt, &slot) && slot >= 1 && slot <=4)
        {
          printf("#PROG-COPY-FLASH-TO-SLOT %d  STARTED\r\n", slot);
          int ret = programmer_copy_flash_to_application(slot); 
          printf("#PROG-COPY-FLASH-TO-SLOT %d  DONE: %d\r\n", slot, ret);
        }
        else
        {
          printf("#ERR: Bad slot: %d\r\n",slot); 
          return 1; 
        }
      }
      if (prefix_matches(in,"#PROG-COPY-SLOT-TO-FLASH"))
      {
#ifndef _BOOTLOADER_

          printf("#ERR: #PROG-COPY-SLOT-TO-FLASH only available on bootlader\r\n");
#else
        int slot =-1; 
        const char * nxt = 0; 
        if (!parse_int(in + sizeof("#PROG-SLOT-TO-FLASH"), &nxt, &slot) && slot >= 1 && slot <=4)
        {
          int ret = programmer_copy_application_to_flash(slot); 
          last_slot = slot; 
          printf("#PROG-COPY-SLOT-TO-FLASH %d  STARTED: %d\r\n", slot, ret);
          program_state = PROGRAM_COPYING_TO_FLASH; 
        }
        else
        {
          printf("#ERR: Bad slot: %d\r\n",slot); 
          return 1; 
        }
#endif
      }
 
      else if (prefix_matches(in,"#PROG-HEXDUMP")) 
      {
        const char * nxt = 0; 
        int slot; 
        int offset; 
        int len; 
        if (!parse_int(in + sizeof("#PROG-HEXDUMP"), &nxt, &slot)
            && !parse_int(nxt,&nxt,&offset)
            && !parse_int(nxt,&nxt,&len))
        {
          printf("#PROG-HEXDUMP(slot=%d,offset=%d,len=%d)\r\n", slot, offset, len); 
          if (slot > 0) 
          {
            wakeup_spi_flash(); 
            spi_flash_application_seek(slot, offset); 
          }
          uint8_t buf[32]; 

          for (int i = 0; i < len; i+=32) 
          {
            int howmany = i + 32  > len ? len-i : 32; 
            if (slot > 0) spi_flash_application_read(slot, howmany, buf); 
            else flash_read(&FLASH, (uint32_t) &__rom_start__+i,  buf, howmany); 
            printf("# %04x ", offset + i); 
            for (int j = 0; j < howmany; j++) 
            {
              printf(" %02x", buf[j]); 
            }
            printf("\r\n"); 
          }
          if (slot > 0) 
            maybe_put_down_spi_flash(); 
        }
      }
      else if (prefix_matches(in,"#PROG-WRITE"))
      {
        const char * nxt = 0; 
        int slot , offset, len; 
        if (!parse_int(in + sizeof("#PROG-WRITE"), &nxt,&slot)
            && !parse_int(nxt,&nxt,&offset)
            && !parse_int(nxt,&nxt,&len)
            )
        {
          while(*nxt==' ') nxt++; 
          int left =  in_len - (nxt -in); 
          if (slot < MIN_WRITE_SLOT || slot > 4) 
          {
            printf("#ERR: Bad slot %d\r\n",slot); 
            return 1;
          }

          uint8_t *decoded = (uint8_t*) nxt; 
          int converted = base64_decode(left, decoded);
          if (converted!=len) 
          {
            printf("#ERR: expected length %d!= decoded length %d\r\n" , len, converted); 
            return -1; 
          }

#ifdef _BOOTLOADER_ 
          if (slot == 0) 
          {
            flash_append(&FLASH, &__rom_start__ + offset, decoded, converted); 
          }
#endif
          if (slot > 0 && slot <=4) 
          {
            wakeup_spi_flash(); 
            spi_flash_application_seek(slot, offset); 
            spi_flash_application_write(slot, converted, decoded); 
            maybe_put_down_spi_flash(); 
          }
          printf("#PROG-WRITE(slot=%d,offset=%d,len=%d)\r\n", slot, offset, len); 
        }
        else
        {
          return 1;
        }
      }
      else if (prefix_matches(in,"#PROG-ERASE"))
      {
        const char * nxt = 0; 
        int slot , offset, len; 
        if (!parse_int(in + sizeof("#PROG-ERASE"), &nxt,&slot)
            && !parse_int(nxt,&nxt,&offset)
            && !parse_int(nxt,&nxt,&len)
            )
        {

          if (len > (int) &__rom_size__  || len < 0) len = (int) &__rom_size__; 

          if (slot < MIN_WRITE_SLOT || slot > 4) 
          {
            printf("#ERR: Bad slot %d\r\n",slot); 
            return 1;
          }

          if (len == 0) 
          {
            printf("#PROG-ERASE(slot=%d,offset=%d,len=%d)\r\n", slot, offset, len); 
            return 0; 
          }



          if (offset & 0xfff || len & 0xfff)
          {
             printf("#ERR: Offset and length must be page aligned\r\n"); 
             return 1; 
          }


          int ret = 0; 
 
#ifdef _BOOTLOADER_ 
          if (slot == 0) 
          {
           if (flash_status == FLASH_BUSY) 
            {
              printf("#ERR: Flash Busy\r\n"); 
              return 1; 
            }
            //ok that this is synchronous
            ret = flash_erase(&FLASH, &__rom_start__ + offset, len >> 12); 
          }
#endif
          if (slot > 0 && slot <=4) 
          {
            wakeup_spi_flash(); 

            //do synchronous, for now. Can make asynchronous later if necessary
            ret = spi_flash_application_erase_sync(slot,offset,len >> 12); 


            maybe_put_down_spi_flash(); 
          }

          printf("#PROG-ERASE(slot=%d,offset=%d,len=%d): %d\r\n", slot, offset, len, ret); 

        }
        else
        {
          printf("#ERR: Trouble parsing %s: %d,%d,%d\r\n",in,slot,offset,len); 
          return 1; 
        }
      }
      else if (prefix_matches(in,"#PROG-READ"))
      {
        const char * nxt = 0; 
        int slot , offset, len; 
        if (!parse_int(in + sizeof("#PROG-READ"), &nxt,&slot)
            && !parse_int(nxt,&nxt,&offset)
            && !parse_int(nxt,&nxt,&len)
            )
        {
          printf("#PROG-READ(slot=%d,offset=%d,len=%d): ", slot, offset, len); 
          if (slot < 0 || slot > 4) 
          {
            printf("#ERR: Bad slot %d\r\n",slot); 
            return 1;
          }

          if (slot > 0) 
          {
            wakeup_spi_flash(); 
            spi_flash_application_seek(slot, offset); 
          }
          uint8_t buf[48]; 

          for (int i = 0; i < len; i+=48) 
          {
            int howmany = i + 48  > len ? len-i : 48; 
            if (slot > 0) spi_flash_application_read(slot, howmany, buf); 
            else flash_read(&FLASH, (uint32_t) &__rom_start__+i+offset,  buf, howmany); 
            base64_print(SBC_UART_DESC, howmany, buf); 
          }
          if (slot > 0) 
            maybe_put_down_spi_flash(); 
 
          printf("\r\n"); 
        }
      }
  return 0; 
}







int programmer_process() 
{
  switch (program_state)
  {
    case PROGRAM_IDLE: 
    case PROGRAM_DONE: 
      maybe_put_down_spi_flash(); 
      program_state = PROGRAM_IDLE; 
      return 0; 
    case PROGRAM_ERROR: 
      printf("#PROG ERR\n"); 
      program_state = PROGRAM_IDLE; 
      return -1;
#ifdef _BOOTLOADER_
    case PROGRAM_COPYING_TO_FLASH: 
      if (!programmer_copy_application_to_flash(last_slot)) 
      {
        last_slot=-1; 
        program_state = PROGRAM_DONE; 
      }
#endif 
   default: 
      break; 
  }

  return 1; 
}


int programmer_copy_flash_to_application(int slot) 
{

  int i=0;

  uint8_t* offset = (uint8_t*) &__rom_start__; 
  int N = (int) &__rom_size__; 
  wakeup_spi_flash(); 
  spi_flash_application_seek(slot,0); 

  uint8_t buf[256]; 

  //try doing it all in one go for now? split it up if we need to. 
  while (i < N) 
  {
    int howmany = i + 256 < N ? 256 : N -i; 
    flash_read(&FLASH, (uint32_t) offset+i, buf, howmany); 
    int wrote = spi_flash_application_write(slot,howmany, buf); 
    i+=wrote; 
    delay_us(100); 
  }
  
  put_down_spi_flash(); 
  return 0; 
}


int programmer_copy_application_to_flash(int slot) 
{
#ifndef _BOOTLOADER_ 
  (void) slot; 
  return -1; 
#else
  int i = 0; 
  uint8_t* offset = (uint8_t*) &__rom_start__; 
  int N = (int) &__rom_size__; 
  spi_flash_application_seek(slot,0); 
  wakeup_spi_flash();

  uint8_t flash_buffer[256]; 

  //do it all in one go 
  while (i < N) 
  {
    int howmany = i + 256 < N ? 256 : N-i; 
    spi_flash_application_read(slot,howmany, flash_buffer); 
    flash_write(&FLASH, offset+i,flash_buffer,howmany); 
    i+=howmany; 
  }
  
  put_down_spi_flash(); 
  printf("#PROG-COPY-SLOT-TO-FLASH %d  DONE!\r\n", slot);
  return 0; 
#endif 
}


