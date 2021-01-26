#include "application/sbc.h" 
#include "application/gpio_expander.h" 
#include "application/lte.h" 
#include "shared/driver_init.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "include/rno-g-control.h" 
#include "application/i2cbus.h" 

#define SBC_BUF_LEN 256
#define SBC_CONSOLE_BUF_LEN 512

ASYNC_READ_BUFFER(SBC_BUF_LEN, sbc); 
#ifndef _DEVBOARD_
ASYNC_READ_BUFFER(SBC_CONSOLE_BUF_LEN, sbc_console); 
#endif

static sbc_state_t state; 

void sbc_init()
{
  /* Initialize main SBC UART reading*/ 
  sbc_uart_read_async(&sbc); 

#ifndef _DEVBOARD_
  sbc_uart_console_read_async(&sbc_console); 

  //figure out if we're on or not 
  i2c_gpio_expander_t i2c_gpio; 
  get_gpio_expander_state(&i2c_gpio,1); //this must be called after i2cbus_init, so we'll have a value; 
  state = i2c_gpio.sbc ? SBC_ON : SBC_OFF; 
#else
  state = SBC_ON; 
#endif
}


static void do_release_boot(const struct timer_task * const task)
{
    gpio_set_pin_direction(SBC_BOOT_SDCARD, GPIO_DIRECTION_IN); 
}

static struct timer_task sbc_release_boot_task = { .cb  = do_release_boot, .interval = 1500, .mode = TIMER_TASK_ONE_SHOT }; 

static void do_turn_on(const struct timer_task * const task)
{
  i2c_gpio_expander_t turn_on_sbc = {.sbc=1}; 
  set_gpio_expander_state (turn_on_sbc,turn_on_sbc); 
  state = SBC_ON; 

  //this means we have to release the boot select switch 
  if (task) 
  {
    timer_add_task(&SHARED_TIMER, &sbc_release_boot_task);
  }
}

//could use the same task for both, I guess? 
static struct timer_task sbc_turn_on_task = { .cb  = do_turn_on, .interval = 500, .mode = TIMER_TASK_ONE_SHOT }; 

int sbc_turn_on(sbc_boot_mode_t boot_mode) 
{
  if (state != SBC_OFF) return -1;  

  if (boot_mode == SBC_BOOT_SDCARD) 
  {
    gpio_set_pin_direction(SBC_BOOT_SDCARD, GPIO_DIRECTION_OUT); 
    gpio_set_pin_level(SBC_BOOT_SDCARD, 0); 
    state = SBC_TURNING_ON; 
    timer_add_task(&SHARED_TIMER, &sbc_turn_on_task);
  }
  else
  {
    do_turn_on(0); 
  }
  return 0; 
}

static int char2val(char c) 
{
  if (c>=0x30 && c <=0x39) return c-0x30; 
  if (c>=0x41 && c <=0x36) return 10+c-0x41; 
  if (c>=0x61 && c <=0x66) return 10+c-0x61; 
  return -1; 
}

//returns 0 on success
static int parse_hex(const char * start, const char **end, uint8_t * byte)
{
  const char * ptr = start;
  //skip leading whitespace
  while (*ptr==' ' || *ptr =='\t') ptr++;

  char first= *ptr++; 
  char second = *ptr; 
  int ok = 0; 

  //check if second is a whitespace or 0, in this case we just have one  
  if (!second  || second == ' ' || second=='\t') 
  {
    int val = char2val(first); 
    if (val < 0) ok = 1; 
    else *byte =val; 
  }
  else
  {
    ptr++; //increment pointer to point after consumed

    int msb = char2val(first); 
    if (msb < 0) 
    {
      ok = 1; 
    }
    else
    {
      int lsb = char2val(second); 
      if (lsb < 0) 
      {
        ok = 1; 
      }
      else
      {
        *byte = (msb<<4)+lsb;
      }
    }
  }

  *end = ptr; 
  return ok; 
}


int sbc_io_process()
{


    //TEMPORARY REALLY DUMB COMMANDS 
    //
    //
    //  Commands start with a # and end with \r\n 
    //  Must not contain \r\n or \0. 
    //
    //  This might be torn out and replaced with something better later 
    //

    //find the next \r\n: 
    
    int dontconsume = 0; 
    int nvalid = 0; 

    while(true) 
    {
      char * ending = strstr((char*) sbc.buf,"\r\n"); 

      //no line ending 
      if (!ending)
      {
        // check if our buffer is full 
        if (sbc.offset == sbc.length) 
        {
          printf("Buffer full! Clearing..\r\n"); 
          async_read_buffer_clear(&sbc);
        }

        //clear the first \0 before the offset, if there is one
        int i = 0; 
        for (int i = 0; i < sbc.offset; i++) 
        {
          if (!sbc.buf[i])
          {
            async_read_buffer_shift(&sbc,i); 
            break; 
          }
        }

        break; 
      }

      //let's set the \r to a 0 so we can use string
      *ending =0;

      int valid = 0; 
      char * in = (char*) sbc.buf+1; 
      //we found a line ending, but we don't start with a #. Skip to end
      if (ending && sbc.buf[0] !='#')
      {

      }

      //now let's try to match commands 
      //First commands with no arguments
      else if (!strcmp(in, "LTE-ON"))
      {
        valid=1; 
        lte_turn_on(); 
        printf("Turning on LTE\r\n"); 
      }
      else if (!strcmp(in,"LTE-ON"))
      {
        valid =1; 
        lte_turn_off(); 
        printf("Turning off LTE\r\n"); 
      }
      else if (!strcmp(in,"RADIANT-ON"))
      {
        valid=1; 
        i2c_gpio_expander_t turn_on_radiant = {.radiant = 1}; 
        set_gpio_expander_state(turn_on_radiant, turn_on_radiant); 
        printf("Turning on RADIANT\r\n"); 
      }

      else if (!strcmp(in,"RADIANT-OFF") )
      {
        valid=1;
        i2c_gpio_expander_t turn_off_radiant = {0}; 
        i2c_gpio_expander_t turn_off_radiant_mask = {.radiant = 1}; 
        set_gpio_expander_state(turn_off_radiant, turn_off_radiant_mask); 
        printf("Turning off RADIANT\r\n"); 
      }
      else if (strstr(in,"I2C-WRITE"))
      {
        static i2c_task_t write_task  = {.addr=0,.write=1,.reg=0,.data=0,.done=1};

        //check if we are ready, otherwise break out of this loop 
        if (!write_task.done) 
        {
          dontconsume=1; 
          valid =1; 
          break;
        }

        const char * nxt = 0; 
        uint8_t addr; 
        uint8_t reg; 
        uint8_t data; 
        valid=1; 
        if (parse_hex(in+sizeof("I2C-WRITE"),&nxt, &addr) ||
            parse_hex(nxt,&nxt,&reg) ||
            parse_hex(nxt,&nxt,&data))
        {
          printf("Failed to interpret #%s\r\n",in); 
        }
        else
        {
          write_task.addr=addr;
          write_task.reg=reg;
          write_task.data=data;
          i2c_enqueue(&write_task); 
          printf("Writing 0x%x to reg 0x%x at addr 0x%x\r\n", data,reg,addr); 
        }
      }
      else if (strstr(in,"I2C-READ"))
      {
        const char *nxt = 0; 
        uint8_t addr;
        uint8_t reg;
        if (parse_hex(in+sizeof("I2C-READ"),&nxt, &addr) ||
            parse_hex(nxt,&nxt,&reg) )
        {
          printf("Failed to interpret #%s\r\n",in);
          valid=1; 
        }
        else
        {
          i2c_task_t read_task = {.addr=addr, .write=0, .reg=reg}; 
          i2c_enqueue(&read_task); 
          while (!read_task.done); //busy wait 
          printf("#I2C-READ %x %x = %x\r\n", read_task.addr, read_task.reg, read_task.data); 
          valid=3; //only do one of these tasks per process, since they can take a while 
        }
      }

      //consume 
      if (!dontconsume) 
      {
        async_read_buffer_shift(&sbc, ending-(const char*) sbc.buf+2); 
      }

      if (!valid) 
      {
        printf("Unrecognized command: %s\r\n", (char*) sbc.buf); 
      }
      else 
      {
        nvalid+=valid; 
      }

      // only do up to 3 commands at a time! 
      if (dontconsume || nvalid > 3) break; 
    }

    return 0; 
}

sbc_state_t sbc_get_state() { return state; } 


int sbc_turn_off() 
{

  if (state != SBC_ON) return -1; 




  return 0; 

} 
