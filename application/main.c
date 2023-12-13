#include "shared/driver_init.h" 
#include "lorawan/lorawan.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "application/sbc.h" 
#include "hal_ext_irq.h" 
#include "hal_gpio.h" 
#include "application/debug.h" 
#include "application/monitors.h" 
#include "shared/spi_flash.h" 
#include "shared/shared_memory.h" 
#include "application/gpio_expander.h" 
#include "config/config.h" 
#include "application/time.h" 
#include "hal_i2c_m_sync.h" 
#include "application/lte.h" 
#include "application/i2cbus.h" 
#include "application/i2cbusmux.h" 
#include "application/power.h" 
#include "application/lowpower.h" 
#include "application/report.h" 
#include "application/commands.h" 
#include "application/mode.h" 
#include "application/reset.h" 

/** This is still mostly a placeholder right now while testing!!! 
 *
 */ 





static int last_nint = 0; 
static volatile int n_interrupts; 
static volatile int n_nmi; 
static volatile uint8_t crashed = CRASH_UNSET; 
static int last_feed = 0; 

void HardFault_Handler() 
{
  get_shared_memory()->crash_reason = CRASH_HARDFAULT; 
  get_shared_memory()->ncrash++; 
  reset(0); 
}

void WDT_Handler() 
{
  hri_wdt_clear_INTFLAG_EW_bit(WDT); 
  printf("woof woof woof\r\n") ; 
}

void NMI_Handler(void) 
{
  n_nmi++; 
  EIC->NMIFLAG.reg = EIC_NMIFLAG_NMI; 
}

static void interrupt()
{
  n_interrupts++; 
}

static uint64_t nticks = 0; 
static uint64_t wokeup_ticks = 0; 
static uint32_t extra_awake_ticks = 0;

int main(void)
{
	/* Initialize system drivers*/ 
	system_init();

  if (ENABLE_WATCHDOG) 
  {
    if (ENABLE_WATCHDOG_EW) 
    {
      NVIC_EnableIRQ(WDT_IRQn); 
      //attempt to set up the EW interrupt. 
      hri_wdt_wait_for_sync(WDT);
      hri_wdt_write_EWCTRL_EWOFFSET_bf(WDT, 0xa); 
      hri_wdt_write_INTEN_EW_bit(WDT,1); 
    }

    wdt_enable(&INTERNAL_WATCHDOG); 
  }
 
  /* Initialize i2c (also initializes busmux, gpio expanders
   * and gets the state of the gpio expanders */ 
  i2c_bus_init(); 


  /* Initialize SPI flash */ 
  spi_flash_init(); 

  /** Initialize ADC monitors */ 
  monitor_init(); 
 
  sbc_init(); 


  /* Set up an interrupt from the SBC*/ 
#ifndef USE_RADIO_DEBUG
  ext_irq_register(GPIO1, interrupt); 
#endif



  /* Set up LTE */ 
  lte_init(); 

  // read in the config block
  config_block_t * cfg = config_block(); 


  printf("#INFO: BOOTED! Station: %d, version: %s\r\n", cfg->app_cfg.station_number, APP_VERSION); 
  if (get_shared_memory()->crash_reason)
  {
    printf("#WARNING: crash-%d\r\n", get_shared_memory()->crash_reason); 
    crashed = get_shared_memory()->crash_reason; 
    get_shared_memory()->crash_reason = CRASH_UNSET; 
  }

  //enable the calendar
  calendar_enable(&CALENDAR); 

  /** Initialize LoRaWAN */ 
  lorawan_init(1); 

  
  /** Initialize power system monitors */ 
  power_monitor_init(); 


  //figure out current mode
  mode_init(); 



  /** The main control loop */ 

	while (1)
  {
    int up = uptime(); 

    if (ENABLE_WATCHDOG) 
    {
      if (up > last_feed) 
      {
        wdt_feed(&INTERNAL_WATCHDOG); 
        last_feed=up; 
      }
    }

    // Service any messages from the SBC
    sbc_process(up);

    low_power_process(); 
 
    if (!low_power_mode) 
    {
      ///Start with potential inputs 
      //Check if we got an interrupt 
      if (n_interrupts > last_nint) 
      {
       printf("#INFO: number of interrupts now %d (3 to reset) \r\n", ++last_nint); 

       if (n_interrupts >= 3) 
       {
         reset(0); 
       }

      }

      // Service LTE (this does nothing for now) 
      lte_process(up); 
     
      if (nticks >= MODE_CHANGE_MINTICKS) 
      {
        (mode_set(config_block()->app_cfg.wanted_state)); 
      }
    }

    // to allow using the same code for both types, use these macros: 

#ifdef _RNO_G_REV_D
#define BATTERY_VOLTAGE(r) (r->power_monitor.BATv_cV / 100.)
#else
#define BATTERY_VOLTAGE(r) (r->V_batt_div25  / 40.)
#endif


    const RNO_G_REPORT_T * maybe_a_report = report_process(up, &extra_awake_ticks); 

    if (maybe_a_report) 
    {
      // See if we meet/exceed thresholds
      if (!low_power_mode) 
      {
        static int nunder = 0; 
        float turnoff = config_block()->app_cfg.turnoff_voltage; 
        if (turnoff > 0 &&  BATTERY_VOLTAGE(maybe_a_report) < turnoff)
        {
          if (++nunder > 3) 
          {
            mode_set(RNO_G_LOW_POWER_MODE); 
          }
        }
        else
        {
          nunder = 0; 
        }
      }
      else 
      {
        float turnon = config_block()->app_cfg.turnon_voltage; 
        if (turnon > 0 && BATTERY_VOLTAGE(maybe_a_report) > turnon)
        {
          mode_set(RNO_G_NORMAL_MODE); 
        }
      }
    }




    // Service LoRaWAN 
    int cant_sleep = lorawan_process(up); 

    uint8_t lora_len, lora_port, *lora_bytes, lora_flags; 
    while (lorawan_rx_peek(&lora_len, &lora_port, &lora_bytes, &lora_flags))
    {
      printf("#LORA_RECV(len=%u,port=%u,flags=%u): ", lora_len, lora_port, lora_flags ); 
      for (uint8_t i = 0; i < lora_len ; i++) 
      {
        printf("%02x ", lora_bytes[i]); 
      }
      printf("\r\n"); 
      if (lora_port > RNO_G_CMD_TOO_SMALL && lora_port < RNO_G_CMD_TOO_LARGE) 
      {
        commands_put(lora_port, lora_len, lora_bytes); 
      }


      lorawan_rx_pop(); 
    }
    
   
    if (ENABLE_WATCHDOG) 
    {
      if (up > last_feed) 
      {
        wdt_feed(&INTERNAL_WATCHDOG); 
        last_feed=up; 
      }
    }

    if (nticks==30) 
    {
      /** Reset reset counter if we made it through the loop 30 times*/ 
      get_shared_memory()->nresets = 0; 
    }


    // See if we can sleep 
    if (!cant_sleep && low_power_mode && (nticks-wokeup_ticks) >= LOW_POWER_AWAKE_TICKS + extra_awake_ticks && sbc_get_state() == SBC_OFF) 
    {

#ifdef _RNO_G_REV_D
      //make sure the vicor is off! 
      low_power_mon_off(); 
#endif


      //feed the watchdog before we sleep 
      wdt_feed(&INTERNAL_WATCHDOG); 
      last_feed=up; 

      wokeup_ticks = ++nticks; 
      extra_awake_ticks = 0; 
      low_power_sleep_for_a_while(LOW_POWER_SLEEP_AMOUNT); 
    }
    else
    {
      delay_ms(DELAY_MS); 
      nticks++; 
    }
	}
}
