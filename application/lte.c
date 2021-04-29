#include "lte.h" 
#include "hal_gpio.h" 
#include "shared/driver_init.h" 
#include "hal_usart_async.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "application/time.h" 
#include "config/config.h" 
#include "include/rno-g-control.h" 
#include "application/mode.h" 

ASYNC_TOKENIZED_BUFFER(128, lte_io,"\r\n", LTE_UART_DESC); 

static lte_state_t lte_state = LTE_INIT; 

lte_state_t lte_get_state() { return lte_state; } 


enum 
{
  LTE_RFSTS=1, 
  LTE_COPS=2, 
  LTE_MONI=3, 
} e_lte_request; 



static void lte_request(int what)
{
  switch (what) 
  {
    case LTE_RFSTS:
      dprintf(LTE_UART_DESC,"AT#RFSTS\r\n"); 
      break;
    case LTE_COPS:
      dprintf(LTE_UART_DESC,"AT+COPS?\r\n"); 
      break;
     case LTE_MONI: 
      dprintf(LTE_UART_DESC,"AT#MONI\r\n"); 
      break; 
     default: 
      break; 
  }
}

static void lte_delayed_rfsts(const struct timer_task * const task) 
{
  (void)task; 
  lte_request(LTE_RFSTS); 
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
  if (force!=1 && lte_state != LTE_OFF) 
  {
    return -1; 
  }

  if (force!=2 && mode_query()!= RNO_G_NORMAL_MODE)
  {
    return -2; 
  }

  gpio_set_pin_direction(LTE_REG_EN,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_REG_EN,1);

  gpio_set_pin_direction(LTE_ON_OFF, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  lte_state = LTE_TURNING_ON; 


  timer_add_task(&SHARED_TIMER, &lte_turn_on_task);
  return 0; 

}

static rno_g_lte_stats_t lte_stats; 


int lte_process()
{

  static int icall = 0; 


  //periodically check RFSTS
  if(lte_state == LTE_ON) 
  {
    if(!(icall & ABOUT_10_SECONDS))
    {
      lte_request(LTE_RFSTS); 
    }

    while (async_tokenized_buffer_ready(&lte_io))
    {
      //Parse RFSTS 
      if (prefix_matches((char*) lte_io.buf,"#RFSTS:"))
      {

        lte_stats.when = get_time(); 
        //RFSTS: <PLMN>,<EARFCN>,<RSRP>,<RSSI>,<RSRQ>,<TAC>,<RAC>,[<TXPWR>],<DRX> ,<MM >,<RRC>,<CID>,<IMSI>,[<NetNameAsc>],<SD>,<ABND>,<T3402>,<T3412>
        // find first comma, make it null 
        char * comma = strchr((char*) lte_io.buf,','); 
        *comma = 0; 
        //see if there is a quote 
        const char * start = strchr((char*) lte_io.buf,'"'); 

        if (!start) 
        {
          lte_stats.mcc=-1;
          lte_stats.mnc=-1; 
        }
        else
        {
          start++; 
          int mcc=-1,mnc=-1; 
          parse_int(start, &start, &mcc);
          parse_int(start, &start, &mnc);
          lte_stats.mcc = mcc; 
          lte_stats.mnc = mnc; 
        }

        //now loop over the number of commas
        for (int i=0; i < 15; i++) 
        {
          start = comma+1; 
          comma = strchr(start,','); 
          *comma =0; 
          if (i ==0 || i == 1 || i == 2 || i==6||i==13||i==14)
          {
            int val = 0; 
            if (comma!=start) 
            {
              parse_int(start,0,&val); 
            }

            if (i ==0) lte_stats.earfcn = val;
            else if (i ==1) lte_stats.rsrp = val;
            else if (i ==2) lte_stats.rssi = val;
            else if (i ==6) lte_stats.tx_power = val;
            else if (i ==13) lte_stats.service_domain = val;
            else if (i ==14) lte_stats.band = val;
          }
          else if (i==3) 
          {
            if (comma == start) 
            {
              lte_stats.neg_rsrq_x10=255; 
            }
            else
            {
              int integ=0, frac=0;
              parse_int(start,&start,&integ); 
              parse_int(start+1,0,&frac); 
              lte_stats.neg_rsrq_x10 = -integ*10+frac; 
            }
          }
        }
      }
      async_tokenized_buffer_discard(&lte_io); 
    }
  }

  //check for rfstats 


  icall++; 

  return 0; 
}

const rno_g_lte_stats_t * lte_get_stats() { return &lte_stats; } 


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
