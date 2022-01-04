#include "lte.h" 
#include "hal_gpio.h" 
#include "shared/driver_init.h" 
#include "hal_usart_async.h" 
#include "shared/io.h" 
#include "shared/spi_flash.h" 
#include "shared/printf.h" 
#include "application/time.h" 
#include "lorawan/lorawan.h" 
#include "config/config.h" 
#include "include/rno-g-control.h" 
#include "application/mode.h" 

/**
 *
 *  Some notes:
 *
 *  Right now this is heavily timer based, though it could be refactored into
 *  state machine calls to lte_process with some work (not sure it matters too
 *  much... might improve reliability of other interrupts though if we get rid
 *  of the LTE timer. 
 *
 */ 

#define LTE_INTERVAL(s)  (s * 100)
#define LTE_MS_INTERVAL(ms)  (ms/10)

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



static struct timer_task lte_delayed_rfsts_task = {.cb = lte_delayed_rfsts, .interval=LTE_INTERVAL(3), .mode = TIMER_TASK_ONE_SHOT}; 


static void lte_delayed_setup_led(const struct timer_task * const task) 
{
  (void) task; 
  dprintf(LTE_UART_DESC,"AT#SLED=2\r\n"); 
  timer_add_task(&SHARED_TIMER, &lte_delayed_rfsts_task);
}

static struct timer_task lte_delayed_setup_led_task = {.cb = lte_delayed_setup_led, .interval=LTE_INTERVAL(2), .mode = TIMER_TASK_ONE_SHOT}; 

static void lte_setup_gpio(const struct timer_task * const task) 
{
  (void) task; 
  dprintf(LTE_UART_DESC,"AT#GPIO=1,0,2,1\r\n"); 
  timer_add_task(&SHARED_TIMER, &lte_delayed_setup_led_task);
}


static struct timer_task lte_setup_gpio_task = {.cb = lte_setup_gpio, .interval=LTE_INTERVAL(5), .mode = TIMER_TASK_ONE_SHOT}; 

static void lte_turn_on_cb(const struct timer_task * const task)
{
  (void) task; 
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  lte_state = LTE_ON;
  timer_add_task(&SHARED_TIMER, &lte_setup_gpio_task);
}

static struct timer_task lte_turn_on_task = { .cb  = lte_turn_on_cb, .interval = LTE_MS_INTERVAL(5500), .mode = TIMER_TASK_ONE_SHOT }; 


static void lte_power_off_cb(const struct timer_task * const task) 
{
  (void) task; 
  gpio_set_pin_level(LTE_REG_EN,0);
  lte_state = LTE_OFF; 
}
static struct timer_task lte_power_off_task = { .cb  = lte_power_off_cb, .interval = LTE_INTERVAL(5), .mode = TIMER_TASK_ONE_SHOT }; 

static void lte_turn_off_cb(const struct timer_task * const task)
{
  (void) task; 
  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_IN);
  timer_add_task(&SHARED_TIMER, &lte_power_off_task);
}

static struct timer_task lte_turn_off_task = { .cb  = lte_turn_off_cb, .interval = LTE_INTERVAL(3), .mode = TIMER_TASK_ONE_SHOT }; 


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
  lte_io_deinit(); 
  //  we are not on
  lte_state = LTE_OFF; 
}

static struct timer_task lte_check_on_task = {.cb = lte_check_on_cb, .interval = LTE_MS_INTERVAL(300), .mode=TIMER_TASK_ONE_SHOT }; 

int lte_init() 
{
  //check to see if we're on... 
  lte_io_init(); 
  timer_add_task(&SHARED_TIMER, &lte_check_on_task);
  dprintf(LTE_UART_DESC,"AT\r\n"); 
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
  lte_io_init(); 
  timer_add_task(&SHARED_TIMER, &lte_turn_on_task);
  return 0; 

}

static rno_g_lte_stats_t lte_stats; 


int lte_process(int up)
{

  static int icall = 0; 


  //periodically check RFSTS 
  if(lte_state == LTE_ON) 
  {
    static int next_rfsts = 20; 
    if( up > next_rfsts)
    {
      lte_request(LTE_RFSTS); 
      int interval = config_block()->app_cfg.lte_stats_interval; 
      if (interval < 10) interval = 10; 
      next_rfsts = up + interval; 
    }

    while (async_tokenized_buffer_ready(&lte_io))
    {
      //Parse RFSTS 
      if (prefix_matches((char*) lte_io.buf,"#RFSTS:"))
      {

        memset(&lte_stats,0,sizeof(lte_stats)); 
        lte_stats.when = get_time(); 
        //RFSTS: <PLMN>,<EARFCN>,<RSRP>,<RSSI>,<RSRQ>,<TAC>,<RAC>,[<TXPWR>],<DRX> ,<MM >,<RRC>,<CID>,<IMSI>,[<NetNameAsc>],<SD>,<ABND>,<T3402>,<T3412>
        // find first comma, make it null 
        char * comma = strchr((char*) lte_io.buf,','); 
        if(!comma) continue; 
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
          lte_stats.parsed_ok++; 
        }

        //now loop over the number of commas
        for (int i=0; i < 15; i++) 
        {
          start = comma+1; 
          comma = strchr(start,','); 
          if (!comma) break; 
          *comma =0; 
          if (i ==0 || i == 1 || i == 2 || i==6||i==13||i==14)
          {
            int val = 0; 
            if (comma!=start) 
            {
              parse_int(start,0,&val); 
              lte_stats.parsed_ok++; 
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
              lte_stats.parsed_ok++; 
              int integ=0, frac=0;
              parse_int(start,&start,&integ); 
              parse_int(start+1,0,&frac); 
              lte_stats.neg_rsrq_x10 = -integ*10+frac; 
            }
          }
        }
        if (lorawan_state() == LORAWAN_READY)
        {
          lorawan_tx_copy(RNO_G_LTE_STATS_SIZE ,RNO_G_MSG_LTE_STATS , (uint8_t*) lte_get_stats(),0); 
        }
      }
      async_tokenized_buffer_discard(&lte_io); 
    }
  }

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

  lte_io_deinit(); 

  gpio_set_pin_direction(LTE_ON_OFF,GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LTE_ON_OFF,0); 
  lte_state = LTE_TURNING_OFF; 
  timer_add_task(&SHARED_TIMER, &lte_turn_off_task);

  return 0 ;
}

#ifndef _RNO_G_REV_D
static void lte_finish_reset_cb(const struct timer_task  *const  task) 
{
  (void) task; 
  gpio_set_pin_direction(LTE_NRST,GPIO_DIRECTION_OFF); 
  lte_state = LTE_ON; // ??? 
}

static struct timer_task lte_reset_task = {.cb = lte_finish_reset_cb, .interval=LTE_MS_INTERVAL(250), .mode = TIMER_TASK_ONE_SHOT}; 



#endif
static void lte_power_cycle(const struct timer_task * const task) 
{
  (void) task; 
  lte_turn_on(1); 
}

static struct timer_task lte_power_cycle_task = {.cb = lte_power_cycle, .interval = LTE_INTERVAL(15), .mode = TIMER_TASK_ONE_SHOT}; 

int lte_reset(int type)
{

  if (type < 0 || type >= LTE_NOT_A_RESET) return -1; 

  if (type == LTE_FACTORY_RESET) 
  {
#ifdef _RNO_G_REV_D
    return -1; 
#else 
    lte_state = LTE_RESETTING; 
    gpio_set_pin_level(LTE_NRST,0); 
    gpio_set_pin_direction(LTE_NRST,GPIO_DIRECTION_OUT); 
    timer_add_task(&SHARED_TIMER, &lte_reset_task); 
#endif
  }

  else if (type == LTE_SOFT_CYCLE) 
  {
    dprintf(LTE_UART_DESC, "ATZ\r\n"); 
  }
  else if (type == LTE_HARD_CYCLE)
  {
    dprintf(LTE_UART_DESC, "AT#ENHRST=1,0\r\n"); 
  }
  else if (type == LTE_POWER_CYCLE) 
  {
    lte_turn_off(1); 
    timer_add_task(&SHARED_TIMER, &lte_power_cycle_task);
  }
  else
  {
    return -1; 
  }
  return 0; 
}
