#include "lte.h" 
#include "hal_gpio.h" 
#include "shared/driver_init.h" 
#include "hal_usart_async.h" 
#include "shared/io.h" 
#include "shared/printf.h" 

ASYNC_READ_BUFFER(256, lte_io); 

static lte_state_t state; 


lte_state_t lte_get_state() { return state; } 



static void lte_turn_on_cb(const struct timer_task * const task)
{
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_UART_ENABLE,0); 
  lte_uart_read_async(&lte_io); //nbd if this is called multiple times 
  state = LTE_ON;
}

static struct timer_task lte_turn_on_task = { .cb  = lte_turn_on_cb, .interval = 5000, .mode = TIMER_TASK_ONE_SHOT }; 

static void lte_turn_off_cb(const struct timer_task * const task)
{
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  gpio_set_pin_level(LTE_REG_EN,0);
  state = LTE_OFF; 
}

static struct timer_task lte_turn_off_task = { .cb  = lte_turn_off_cb, .interval = 3000, .mode = TIMER_TASK_ONE_SHOT }; 

int lte_init() 
{
  //nothing right now 
  return 0; 
}

int lte_turn_on()
{
  if (state != LTE_OFF) 
  {
    return -1; 
  }

  gpio_set_pin_direction(LTE_REG_EN,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_REG_EN,1);

  gpio_set_pin_direction(LTE_ON_OFF, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  state = LTE_TURNING_ON; 


  timer_add_task(&SHARED_TIMER, &lte_turn_on_task);
  return 0; 

}

int lte_process()
{



  return 0; 
}




int lte_turn_off()
{
  if (state != LTE_ON) 
  {
    return -1; 
  }

  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_IN);
  usart_async_disable(&LTE_UART); 


  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  state = LTE_TURNING_OFF; 
  timer_add_task(&SHARED_TIMER, &lte_turn_off_task);

  return 0 ;
}
