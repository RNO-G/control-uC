#include "application/heater.h" 
#include "shared/driver_init.h" 

int heater_on(void)
{
#ifndef _RNO_G_REV_D
 gpio_set_pin_direction(HEATER_FET_CNTRL, GPIO_DIRECTION_OUT); 
 gpio_set_pin_level(HEATER_FET_CNTRL, true);
 return 0; 
#else
 return 1; 
#endif
}

int heater_off(void)
{
#ifndef _RNO_G_REV_D
 gpio_set_pin_level(HEATER_FET_CNTRL, false);
 gpio_set_pin_direction(HEATER_FET_CNTRL, GPIO_DIRECTION_OFF); 
 return 0; 
#else
 return 1; 
#endif
}
