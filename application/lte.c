#include "lte.h" 
#include "hal_gpio.h" 
#include "shared/driver_init.h" 
#include "hal_usart_async.h" 
#include "shared/io.h" 
#include "shared/printf.h" 

ASYNC_TOKENIZED_BUFFER(128, lte_io,"\r\n", LTE_UART_DESC); 

static lte_state_t lte_state; 

lte_state_t lte_get_state() { return lte_state; } 





static void lte_setup_gpio() 
{
  dprintf(LTE_UART_DESC,"AT+GPIO=1,0,2,1\r\n"); 
}

static void lte_delayed_setup_led(const struct timer_task * const task) 
{
  (void) task; 
  dprintf(LTE_UART_DESC,"AT+SLED=2\r\n"); 
}

static struct timer_task lte_delayed_setup_led_task = {.cb = lte_delayed_setup_led, .interval=20, .mode = TIMER_TASK_ONE_SHOT}; 


static void lte_turn_on_cb(const struct timer_task * const task)
{
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_UART_ENABLE,0); 
  lte_state = LTE_ON;
  lte_setup_gpio(); 
  timer_add_task(&SHARED_TIMER, &lte_delayed_setup_led_task);
}

static struct timer_task lte_turn_on_task = { .cb  = lte_turn_on_cb, .interval = 550, .mode = TIMER_TASK_ONE_SHOT }; 

static void lte_turn_off_cb(const struct timer_task * const task)
{
  (void) task; 
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  gpio_set_pin_level(LTE_REG_EN,0);
  lte_state = LTE_OFF; 
}

static struct timer_task lte_turn_off_task = { .cb  = lte_turn_off_cb, .interval = 300, .mode = TIMER_TASK_ONE_SHOT }; 

int lte_init() 
{
  //nothing right now 
  return 0; 
}

int lte_turn_on(int force)
{
  if (!force && lte_state != LTE_OFF) 
  {
    return -1; 
  }

  gpio_set_pin_direction(LTE_REG_EN,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_REG_EN,1);

  gpio_set_pin_direction(LTE_ON_OFF, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  lte_state = LTE_TURNING_ON; 


  timer_add_task(&SHARED_TIMER, &lte_turn_on_task);
  return 0; 

}

int lte_process()
{



  return 0; 
}




int lte_turn_off(int force)
{
  if (!force && lte_state != LTE_ON) 
  {
    return -1; 
  }

  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_IN);


  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  lte_state = LTE_TURNING_OFF; 
  timer_add_task(&SHARED_TIMER, &lte_turn_off_task);

  return 0 ;
}
