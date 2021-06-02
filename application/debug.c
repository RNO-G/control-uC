
#include "application/debug.h"

#include "atmel_start_pins.h" 
#include "hal_gpio.h" 
#include "shared/printf.h" 


static int vicor_en_pin = VICOR_EN; 

void dbg_gpio_set_pin_level(int pin, int level) 
{
  gpio_set_pin_level(pin,level); 

}

void infinite_loop() 
{
  printf("Entering infinite loop!\r\n"); 
  int icnt = 0; 
  int ibeer = 0; 

  while (1) 
  {
    icnt++; 
    if ((icnt & 0x7fff)==0) 
    {
      printf("%d bottles of beer on the wall...\r\n",ibeer++); 
    }
  }
}



