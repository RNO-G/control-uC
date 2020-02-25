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

//The io device
static struct io_descriptor * io; 


int programmer_enter(const char * cmd,  struct io_descriptor * dev) 
{
  if (program_state) return 1; 
  int check_slot; 
  int which = which_command(cmd,&check_slot); 
  if (!which || check_slot < 0 || check_slot > 4 || (!check_slot && !am_i_bootloader()))
  {
    io_write(dev,"#ERR\n", sizeof("#ERR\n")); 
    return 1; 
  }
  
  i = 0; 
  io = dev; 
  slot = check_slot; 

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
    case PROGRAM_WRITE_CONFIRM; 

    default: 
      break; 
  }

  return 1; 
}




