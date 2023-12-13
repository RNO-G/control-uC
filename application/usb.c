#include "application/usb.h" 
#include "shared/driver_init.h" 



void usb_reset(void) 
{
  gpio_set_pin_level(USBHUB_RESET,0);
  delay_us(5); 
  gpio_set_pin_level(USBHUB_RESET,1);

}
