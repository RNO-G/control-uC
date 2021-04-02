
#include "application/debug.h"

#include "atmel_start_pins.h" 
#include "hal_gpio.h" 


static int vicor_en_pin = VICOR_EN; 

void dbg_gpio_set_pin_level(int pin, int level) 
{
  gpio_set_pin_level(pin,level); 

}
