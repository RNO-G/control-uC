#include "application/sbc.h" 
#include "application/gpio_expander.h" 
#include "application/lte.h" 
#include "shared/driver_init.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "include/rno-g.h" 

ASYNC_READ_BUFFER(256, sbc); 
#ifndef _DEVBOARD_
ASYNC_READ_BUFFER(512, sbc_console); 
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

int sbc_io_process()
{
    //TEMPORARY REALLY DUMB COMMANDS 
    //
    char * where = strstr((char*)sbc.buf, "#LTE-ON\r\n");
    if (where)
    {
      *where=0; 
      async_read_buffer_shift(&sbc, 512); 
      lte_turn_on(); 
      printf("Turning on LTE\\rn"); 
    }

    where = strstr((char*)sbc.buf, "#LTE-OFF\r\n");
    if (where)
    {
      *where=0; 
      async_read_buffer_shift(&sbc, 512); 
      lte_turn_off(); 
      printf("Turning off LTE\r\n"); 
    }

    where = strstr((char*)sbc.buf,"#RADIANT-ON\r\n"); 
    if (where) 
    {
      *where = 0; 
      async_read_buffer_shift(&sbc,512); 
      i2c_gpio_expander_t turn_on_radiant = {.radiant = 1}; 
      set_gpio_expander_state(turn_on_radiant, turn_on_radiant); 
      printf("Turning on RADIANT\r\n"); 
    }

    where = strstr((char*)sbc.buf,"#RADIANT-OFF\r\n"); 
    if (where) 
    {
      *where = 0; 
      async_read_buffer_shift(&sbc,512); 
      i2c_gpio_expander_t turn_off_radiant = {0}; 
      i2c_gpio_expander_t turn_off_radiant_mask = {.radiant = 1}; 
      set_gpio_expander_state(turn_off_radiant, turn_off_radiant_mask); 
      printf("Turning off RADIANT\r\n"); 
    }

    return 0; 
}

sbc_state_t sbc_get_state() { return state; } 


int sbc_turn_off() 
{

  if (state != SBC_ON) return -1; 




  return 0; 

} 
