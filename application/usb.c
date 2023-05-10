#include "application/usb.h" 
#include "shared/driver_init.h" 



void usb_reset(void) 
{
  gpio_set_pin_direction(USBHUB_RESET,GPIO_DIRECTION_OUT);
  delay_us(5); 
  gpio_set_pin_direction(USBHUB_RESET,GPIO_DIRECTION_IN);

}
