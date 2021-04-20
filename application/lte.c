#include "lte.h" 
#include "hal_gpio.h" 
#include "shared/driver_init.h" 
#include "hal_usart_async.h" 
#include "shared/io.h" 
#include "shared/printf.h" 

ASYNC_TOKENIZED_BUFFER(128, lte_io,"\r\n", LTE_UART_DESC); 

static lte_state_t lte_state = LTE_INIT; 

lte_state_t lte_get_state() { return lte_state; } 



static void lte_delayed_rfsts(const struct timer_task * const task) 
{
  //for debugger only :)
  if (((int) task) == 1) dprintf(LTE_UART_DESC,"AT+COPS?\r\n"); 
  else dprintf(LTE_UART_DESC,"AT#RFSTS\r\n"); 
}



static struct timer_task lte_delayed_rfsts_task = {.cb = lte_delayed_rfsts, .interval=300, .mode = TIMER_TASK_ONE_SHOT}; 


static void lte_delayed_setup_led(const struct timer_task * const task) 
{
  (void) task; 
  dprintf(LTE_UART_DESC,"AT#SLED=2\r\n"); 
  timer_add_task(&SHARED_TIMER, &lte_delayed_rfsts_task);
}

static struct timer_task lte_delayed_setup_led_task = {.cb = lte_delayed_setup_led, .interval=200, .mode = TIMER_TASK_ONE_SHOT}; 

static void lte_setup_gpio(const struct timer_task * const task) 
{
  (void) task; 
  dprintf(LTE_UART_DESC,"AT#GPIO=1,0,2,1\r\n"); 
  timer_add_task(&SHARED_TIMER, &lte_delayed_setup_led_task);
}


static struct timer_task lte_setup_gpio_task = {.cb = lte_setup_gpio, .interval=500, .mode = TIMER_TASK_ONE_SHOT}; 

static void lte_turn_on_cb(const struct timer_task * const task)
{
  (void) task; 
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_UART_ENABLE,0); 
  lte_state = LTE_ON;
  timer_add_task(&SHARED_TIMER, &lte_setup_gpio_task);
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


static void lte_check_on_cb(const struct timer_task * const task)
{
  (void) task; 
  while(async_tokenized_buffer_ready(&lte_io))
  {
    async_tokenized_buffer_discard(&lte_io); 
    if (!strcmp((char*) lte_io.buf,"OK"))
    {
      lte_state = LTE_ON; 
      //note: the SBC must be keeping this up, but let's not count on it 
      gpio_set_pin_direction(LTE_REG_EN,GPIO_DIRECTION_OUT);
      gpio_set_pin_level(LTE_REG_EN,1);
      return; 
    }
  }
  //  we are not on
  lte_state = LTE_OFF; 
  gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_IN);
}

static struct timer_task lte_check_on_task = {.cb = lte_check_on_cb, .interval = 10, .mode=TIMER_TASK_ONE_SHOT }; 

int lte_init() 
{
  //check to see if we're on... 
   gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_UART_ENABLE,0); 
  dprintf(LTE_UART_DESC,"AT\r\n"); 
  timer_add_task(&SHARED_TIMER, &lte_check_on_task);
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
