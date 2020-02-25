#include "shared/programer.h" 
#include "shared/spi_flash.h" 
#include "hal_flash.h" 


static int am_i_bootloader()
{
#ifdef _BOOTLOADER_ 
  return 1; 
#else
  return 0; 
#endif 
}




static enum 
{
  PGM_NO_CMD = 0
  PGM_WRITE_CMD=1,
  PGM_READ_CMD=2
  PGM_ERASE_CMD =3
} commands; 


static int which_command(const char * cmd, int * slot) 
{
  if (cmd [0] != '#') return PGM_NO_CMD; 

  int ret = 0; 
  if ( !memcmp(cmd+1, "PRG@")) ret = PGM_WRITE_CMD; 
  else if ( !memcmp(cmd+1, "DEL@")) ret = PGM_ERASE_CMD; 
  else if ( !memcmp(cmd+1, "REA@")) ret = PGM_READ_CMD; 

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
  PROGRAM_READ_READING
  PROGRAM_ERASE_INIT, 
  PROGRAM_ERASE_CONFIRM, 
  PROGRAM_ERASE_NBLOCKS,
  PROGRAM_ERASE_ERASING, 
  PROGRAM_ERROR,
  PROGRAM_DONE
} program_state; 


static int slot; 

// The byte or block counter 
static int i; 

// The byte or block number
static int N; 


//The io device
static struct io_descriptor * io; 

#define IO_WRITE_LITERAL(x) io_write(dev, x, sizeof(x)-1) 
#define IO_WRITE_STRING(x) io_write(dev, x, strlen(x)) 
#define IO_WRITE_CHAR(x) io_write(dev, x, 1) 

static volatile enum 
{
  FLASH_READY, 
  FLASH_BUSY, 
  FLASH_ERROR
}flash_status ; 


static void flash_ready_callback(struct flash_descriptor * const d) 
{
  flash_status = FLASH_READY; 
}

static void flash_error_callback(struct flash_descriptor * const d) 
{
  flash_status = FLASH_ERROR; 
}

int programmer_enter(const char * cmd,  struct io_descriptor * dev) 
{
  if (program_state) return 1; 
  int check_slot; 
  int which = which_command(cmd,&check_slot); 
  if (!which || check_slot < 0 || check_slot > 4 || (!check_slot && !am_i_bootloader()))
  {
    IO_WRITE("#ERR\n"); 
    return 1; 
  }
  
  i = 0; 
  io = dev; 
  slot = check_slot; 

  if (slot == 0) 
  {
    flash_register_callback(&FLASH, FLASH_CB_READY, flash_ready_callback); 
    flash_register_callback(&FLASH, FLASH_CB_ERROR, flash_error_callback); 
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
  IO_WRITE_LITERAL("?CONFIRM@"); 
  IO_WRITE_CHAR(slot+'0'); 
  IO_WRITE_CHAR('\n'); 
}


const int accum_buf_size = 16; 
const int buf_len = 0;
static char accum_buf[accum_buf_size];  

//Wait until buffer has a \n. Will return true when that happens (or -1 if maximum size exceeded) 
static int accumulate_until_lf(void) 
{
  static int total = 0; 
  
  char c; 
  int got = io_read(dev, &c, 1); 
  if (!got) return 0; 

  if (c == '\n')
  {
    accum_buf[total] = '0; 
    buf_len = total; 
    total = 0; // get ready for next time 
    return 1; 
  }

  accum_buf[total++] = c; 
  if (total == accum_buf_size-1) 
  {
    total = 0; //error state, reset 
    buf_len = 0; 
    return -1; 

  }

  return 0; 
}



static int check_confirm_slot(int success_state) 
{

  if (strlen(accum_buf) == 1 && accum_buf[0] == slot + '0' )
  {
      program_state = success_state; 
      return 1; 
  }
  program_state = PROGRAM_ERROR; 
  return 0; 
}


static int read_nblocks() 
{

  int total = 0; 
  int mult = 1; 
  for (int idig = 0; idig < buf_len; idig++) 
  {
    char c = accum_buf[buf_len-1-idig]; 
    if (c < '0' || c > '9') return 0; 
    total += mult*(c-'0'); 
    mult*=10; 
  }

  N = total; 
  return total; 
}


/* TODO: this is enormously wasteful in terms of memory */ 
static uint8_t flash_buffer[256]; 
static int flash_buffer_i= 0; 
static void reset_flash_buffer() { flash_buffer_i = 0; } 
static int accumulate_flash_buffer() 
{
  if (flash_buffer_i >= 256) return 1; 

  uint8_t c; 
  int got = io_read(dev, &c,1);
  if (!got) return 0; 
  flash_buffer[flash_buffer_i++] = c; 

  if (flash_buffer_i >= 256) return 1; 

  return 0; 
}


static void process_write() 
{


  static int transfer_was_started = 0; 
  static int sent_query = -1; 

  if (flash_status == FLASH_BUSY) 
  {
    return; 
  }

  if (flash_status == FLASH_ERROR) 
  {
    IO_WRITE_LITERAL("#ERR\n"); 
    program_state = PROGRAM_ERROR; 
  }

  

  //this means we are done with the transfer, so we can ask for a new buffer
  if (transfer_was_started)
  {
    transfer_was_started = 0; 
    i++; //move on to the next block

    if (i == N) 
    {
      state = PROGRAM_DONE; 
      return; 
    }
    reset_flash_buffer(); 
  }

  if (sent_query < i);
  {
    IO_WRITE_LITERAL("BLOCK_"); 
    int hundreds = i < 100 ? 0 : i/100; 
    IO_WRITE_CHAR(hundreds-'0'); 
    int tens = (i-hundreds*100) / 10; 
    IO_WRITE_CHAR(tens-'0'); 
    int ones = (i - hundreds*100 - tens*10); 
    IO_WRITE_CHAR(ones-'0'); 
    IO_WRITE_CHAR("\n"); 
    sent_query = i; 
  }

  //accumulate more bytes, if we have 256, then let's pass to flash
  if (accumulate_flash_buffer())
  {

    //otherwise, let's write 256 bytes
    if (slot==0) 
    {
      flash_append(&FLASH, 8*1024 + 256*i, flash_buffer); 
    }
    else
    {
      if (i == 0) spi_flash_application_seek(slot,0); 
      spi_flash_application_write(slot,256, flash_buffer); 
    }

    transfer_was_started = 1;
  }
  return;  

}



int programmer_process() 
{
  switch (program_state)
  {
    case PROGRAM_IDLE: 
    case PROGRAM_DONE: 
      program_state = PROGRAM_IDLE; 
      return 0; 
    case PROGRAM_ERROR: 
      program_state = PROGRAM_IDLE; 
      return -1;
    case PROGRAM_WRITE_INIT; 
      write_confirm(); 
      state = PROGRAM_WRITE_CONFIRM; 
      break; 
    case PROGRAM_WRITE_CONFIRM:  
      accumulate_until_lf() && check_confirm_slot(PROGRAM_WRITE_NBLOCKS) && IO_WRITE_LITERAL("?N_256B_BLOCKS\n"); 
      break; 
    case PROGRAM_ERASE_CONFIRM: 
      accumulate_until_lf() && check_confirm_slot(PROGRAM_ERASE_NBLOCKS) && IO_WRITE_LITERAL("?N_4KB_BLOCKS\n"); 
      break; 
    case PROGRAM_WRITE_NBLOCKS: 
     if (accumulate_until_lf() ==1 && !read_nblocks())
     {
       i = 0; 
       state = PROGRAM_WRITE_WRITING; 
     }
     else 
     {
       state = PROGRAM_ERROR;
     }
    case PROGRAM_WRITE_WRITING:
      process_write() ;
     break; 
    default: 
      break; 
  }

  return 1; 
}




