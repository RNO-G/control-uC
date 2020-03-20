#include "lte.h" 
#include "hal_gpio.h" 
#include "driver_init.h" 
#include "hal_usart_async.h" 
#include "shared/printf.h" 

enum 
{
  LTE_OFF, 
  LTE_TURNING_ON,
  LTE_ON,
  LTE_TURNING_OFF
} lte_state; 


int lte_turn_on()
{
  if (lte_state != LTE_OFF) 
  {
    return -1; 
  }

  gpio_set_pin_direction(LTE_REG_EN,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_REG_EN,1);

  gpio_set_pin_direction(LTE_ON_OFF, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  lte_state = LTE_TURNING_ON; 

  // for now, be extra stupid 
  printf("waiting for lte to turn on like an idiot\n"); 
  delay_ms(5000); 
  printf("it's on now probably\n"); 

  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);

  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_UART_ENABLE,0); 
  usart_async_enable(&LTE_UART); 

  return 0; 

}

int lte_process()
{



}

int lte_turn_off()
{
  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_IN);
  usart_async_disable(&LTE_UART); 


  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  lte_state = LTE_TURNING_OFF; 

  // for now, be extra stupid 
  printf("waiting for lte to turn off like an idiot\n"); 
  delay_ms(2000); 
  printf("it's off now probably\n"); 

  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  gpio_set_pin_level(LTE_REG_EN,0);

}
