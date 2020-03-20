
#include "application/debug.h"

#include "hal_gpio.h" 



void dbg_gpio_set_pin_level(int pin, int level) 
{
  gpio_set_pin_level(pin,level); 

}
