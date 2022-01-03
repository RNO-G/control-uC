#include "application/lowpower.h" 
#include "application/i2cbus.h" 
#include "application/sbc.h" 
#include "hal_calendar.h" 
#include "shared/driver_init.h" 
#include "application/monitors.h" 
#include "shared/spi_flash.h" 
#include "time.h" 
#include "hal_sleep.h" 
#include "application/i2cbusmux.h" 

volatile int low_power_mode = 0; 
volatile int waiting_for_sbc = 0; 

static volatile int woke=0; 
static volatile int vicor_state = 1; 

int low_power_mon_on() 
{
#ifdef _RNO_G_REV_D

  if (!low_power_mode) return 0; 
  if (!vicor_state) 
  {
    i2c_unstick(10); 
    gpio_set_pin_direction(VICOR_EN, GPIO_DIRECTION_IN);
    monitor_init(); 
    vicor_state = 1; 
    delay_ms(15); //wait a bit for things to stabilize 
  }
#endif
  return 0; 
}



int low_power_mon_off() 
{
#ifdef _RNO_G_REV_D
  if (!low_power_mode) return 0; 

  //Don't turn off the vicor if the SBC is still on!!! 
  if (vicor_state && sbc_get_state() == SBC_OFF) 
  {
    i2c_bus_deinit(); 
    gpio_set_pin_direction(VICOR_EN, GPIO_DIRECTION_OUT);
    vicor_state = 0; 
    monitor_deinit(); 
  }
#endif
  return 0;
}




int low_power_mode_enter() 
{
#ifndef _RNO_G_REV_D
  i2c_busmux_quick_select(I2C_BUSMUX_WINTER); 
  gpio_set_pin_level(VICOR_EN, false); 
#else
  low_power_mon_off(); 
#endif
  low_power_mode =1; 
  return 0;
} 



int low_power_mode_exit() 
{
#ifdef _RNO_G_REV_D
  low_power_mon_on(); 
  i2c_unstick(10); //just in case? 
#else
  gpio_set_pin_level(VICOR_EN, true); 
  i2c_busmux_quick_select(I2C_BUSMUX_BOTH); 
#endif
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
#ifdef _RNO_G_REV_D
  low_power_mon_off(); 
#endif
  int cmp = _calendar_get_counter(&CALENDAR.device)+howlong; 
  _calendar_register_callback(&CALENDAR.device,wakeup); 
  _calendar_set_comp(&CALENDAR.device,cmp); 
  woke = 0; 
  spi_flash_deep_sleep(); 
  return sleep(3); 
}
