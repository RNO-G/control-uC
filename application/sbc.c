#include "application/sbc.h" 
#include "application/gpio_expander.h" 
#include "application/lte.h" 
#include "shared/driver_init.h" 
#include "shared/io.h" 
#include "shared/spi_flash.h" 
#include "hal_calendar.h" 
#include "shared/printf.h" 
#include "include/rno-g-control.h" 
#include "application/i2cbus.h" 

//these define the maximum line length! 
#define SBC_BUF_LEN 128 
#define SBC_CONSOLE_BUF_LEN 128

ASYNC_TOKENIZED_BUFFER(SBC_BUF_LEN, sbc,"\r\n", SBC_UART_DESC); 

#ifndef _DEVBOARD_
ASYNC_TOKENIZED_BUFFER(SBC_CONSOLE_BUF_LEN, sbc_console,"\r\n", SBC_UART_CONSOLE_DESC); 
#endif

static sbc_state_t state; 

void sbc_init()
{
#ifndef _DEVBOARD_
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
    (void) task; 
    gpio_set_pin_direction(SBC_BOOT_SDCARD, GPIO_DIRECTION_IN); 
}

static struct timer_task sbc_release_boot_task = { .cb  = do_release_boot, .interval = 150, .mode = TIMER_TASK_ONE_SHOT }; 

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
static struct timer_task sbc_turn_on_task = { .cb  = do_turn_on, .interval = 50, .mode = TIMER_TASK_ONE_SHOT }; 

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

static int parse_int(const char * start, const char **end, int * num) 
{
  if (!num) return 1; 
  const char * ptr = start; 
  int sign = 1; 
  while (*ptr==' ' || *ptr=='\t') ptr++; 

  *num = 0; 
  char first = *ptr; 
  if (first=='-') 
  {
    sign = -1; 
  }
  
  while(*ptr >= '0' && *ptr <='9')
  {
    *num *= 10; 
    *num += (*ptr-'0'); 
    ptr++; 
  }


   if (end) *end = ptr; 
  *num*=sign; 
  return 0; 
}


//returns 0 on success
static int parse_hex(const char * start, const char **end, uint8_t * byte)
{
  if (!byte) return 1; 
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


static int prefix_matches(const char * haystack, const char * prefix) 
{
  int i = 0; 
  while(prefix[i] && haystack[i]) 
  {
    if (haystack[i] != prefix[i]) return 0; 
    i++; 
  }
  return 1; 
}


int sbc_io_process()
{


    // SBC Commands
    //
    //  Commands start with a # and end with \r\n 
    //  Must not contain \r\n or \0. 
    //
    //  This might be torn out and replaced with something better later 
    //
    //  Note that this in addition to the "normal" command processing, that can 
    //  come via LoRaWAN and eventually LTE. 


    //find the next \r\n: 
    
    int dontconsume = 0; 
    int nvalid = 0; 

    while(async_tokenized_buffer_ready(&sbc)) 
    {
      int valid = 0; 
      char * in = (char*) sbc.buf+1; 
      //we don't start with a #. Skip to end
      if (sbc.buf[0] !='#')
      {

      }

      //now let's try to match commands 
      //First commands with no arguments
      else if (!strcmp(in, "LTE-ON"))
      {
        valid=1; 
        lte_turn_on(); 
        printf("#LTE-ON: ACK \r\n"); 
      }
      else if (!strcmp(in,"LTE-ON"))
      {
        valid =1; 
        lte_turn_off(); 
        printf("#LTE-OFF: ACK\r\n"); 
      }
      else if (!strcmp(in,"RADIANT-ON"))
      {
        valid=1; 
        i2c_gpio_expander_t turn_on_radiant = {.radiant = 1}; 
        set_gpio_expander_state(turn_on_radiant, turn_on_radiant); 
        printf("#RADIANT-ON: ACK\r\n"); 
      }

      else if (!strcmp(in,"RADIANT-OFF") )
      {
        valid=1;
        i2c_gpio_expander_t turn_off_radiant = {0}; 
        i2c_gpio_expander_t turn_off_radiant_mask = {.radiant = 1}; 
        set_gpio_expander_state(turn_off_radiant, turn_off_radiant_mask); 
        printf("#RADIANT-OFF: ACK\r\n"); 
      }
      else if (prefix_matches(in,"I2C-WRITE"))
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
          printf("#ERR: Failed to interpret #%s\r\n",in); 
        }
        else
        {
          write_task.addr=addr;
          write_task.reg=reg;
          write_task.data=data;
          i2c_enqueue(&write_task); 
          printf("#I2C_WRITE( val=0x%x,reg=0x%x,addr=0x%x)\r\n", data,reg,addr); 
        }
      }
      else if (prefix_matches(in,"I2C-READ"))
      {
        const char *nxt = 0; 
        uint8_t addr;
        uint8_t reg;
        if (parse_hex(in+sizeof("I2C-READ"),&nxt, &addr) ||
            parse_hex(nxt,&nxt,&reg) )
        {
          printf("#ERR: Failed to interpret #%s\r\n",in);
          valid=1; 
        }
        else
        {
          i2c_task_t read_task = {.addr=addr, .write=0, .reg=reg}; 
          i2c_enqueue(&read_task); 
          while (!read_task.done); //busy wait 
          printf("#I2C-READ: addr(%x) reg(%x) = %x\r\n", read_task.addr, read_task.reg, read_task.data); 
          valid=3; //only do one of these tasks per process, since they can take a while 
        }
      }

      else if (prefix_matches(in,"SET-STATION"))
      {
        const char * nxt = 0; 
        int station; 
        if (!parse_int(in+sizeof("SET-STATION"), &nxt, &station))
        {
          config_block()->app_cfg.station_number= station;
          config_block_sync(); 
          printf("#SET-STATION: %d\r\n", station); 
          valid = 1; 
        }
        else
        {
          printf("#ERR: trouble parsing int"); 
        }
      }
      else if (prefix_matches(in,"SET-GPS-OFFSET"))
      {
        const char * nxt = 0; 
        int offset; 
        if (!parse_int(in+sizeof("SET-GPS-OFFSET"), &nxt, &offset))
        {
          config_block()->app_cfg.gps_offset= offset;
          config_block_sync(); 
          printf("#SET-GPS-OFFSET: %d\r\n", offset); 
          valid = 1; 
        }
        else
        {
          printf("#ERR: trouble parsing int"); 
        }
      }
 
      else if (!strcmp(in,"GET-STATION"))
      {
        printf("#GET-STATION: %d\r\n", config_block()->app_cfg.station_number); 
        valid = 1; 
      }
      else if (!strcmp(in,"FLUSH"))
      {
        flush_buffers(); 
        printf("#FLUSH: ACK\r\n"); 
        valid = 1; 
      }


      else if (!strcmp(in,"NOW"))
      {
        struct calendar_date_time now; 
        calendar_get_date_time(&CALENDAR,&now); 
        valid = 1; 
        printf("#NOW: %d-%02d-%02d %02d:%02d:%02d\n", now.date.year, now.date.month, now.date.day, now.time.hour, now.time.min, now.time.sec); 
      }




      if (!valid) 
      {
        printf("#ERR don't understand: %s\r\n", sbc.buf); 
      }
      else 
      {
        nvalid+=valid; 
      }

      //consume 
      if (!dontconsume) 
      {
        async_tokenized_buffer_discard(&sbc); 
      }
 

     // only do up to 3 commands at a time! 
      if (dontconsume || nvalid > 3) break; 
    }

    return 0; 
}

sbc_state_t sbc_get_state() { return state; } 

__attribute__((section (".keepme")))
int sbc_turn_off() 
{

  if (state != SBC_ON) return -1; 

  //we must hit the power for a bit
  //
  //
  //

  //kill the power
  i2c_gpio_expander_t turn_off_sbc = {.sbc=0}; 
  i2c_gpio_expander_t turn_off_sbc_mask = {.sbc=1}; 
  set_gpio_expander_state(turn_off_sbc,turn_off_sbc_mask); 
  state = SBC_OFF; 

  return 0; 

} 
