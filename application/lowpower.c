#include "application/lowpower.h" 
#include "application/i2cbus.h" 
#include "application/sbc.h" 
#include "hal_calendar.h" 
#include "shared/driver_init.h" 
#include "application/monitors.h" 
#include "shared/spi_flash.h" 
#include "time.h" 
#include "hal_sleep.h" 

volatile int low_power_mode = 0; 
volatile int waiting_for_sbc = 0; 

volatile static int woke=0; 
static volatile int vicor_state = 1; 

int low_power_mon_on() 
{
  if (!low_power_mode) return 0; 
  if (!vicor_state) 
  {
    i2c_unstick(10); 
    gpio_set_pin_direction(VICOR_EN, GPIO_DIRECTION_IN);
    monitor_init(); 
    vicor_state = 1; 
    delay_ms(5); //give us a chance... 
  }
  return 0; 
}



int low_power_mon_off() 
{
  if (!low_power_mode) return 0; 

  //Don't turn off the vicor if the SBC is still on!!! 
  if (vicor_state && sbc_get_state() == SBC_OFF) 
  {
    i2c_bus_deinit(); 
    gpio_set_pin_direction(VICOR_EN, GPIO_DIRECTION_OUT);
    vicor_state = 0; 
    monitor_deinit(); 
  }
  return 0;
}




int low_power_mode_enter() 
{
  low_power_mon_off(); 
  low_power_mode =1; 
  return 0;
} 



int low_power_mode_exit() 
{
  low_power_mon_on(); 
  low_power_mode=0; 
  return 0;
}

static void wakeup() 
{
  woke = 1 ; 
}


int low_power_sleep_for_a_while(int howlong) 
{
  if (howlong <1) return 0; 
  low_power_mon_off(); 
  int cmp = _calendar_get_counter(&CALENDAR)+howlong; 
  _calendar_register_callback(&CALENDAR,wakeup); 
  _calendar_set_comp(&CALENDAR,cmp); 
  woke = 0; 
  spi_flash_deep_sleep(); 
  return sleep(3); 
}
