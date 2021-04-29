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
#include "application/power.h" 
#include "application/lowpower.h" 
#include "application/commands.h" 
#include "application/mode.h" 
#include "application/reset.h" 

/** This is still mostly a placeholder right now while testing!!! 
 *
 */ 





static int last_nint = 0; 
static volatile int n_interrupts; 
static volatile int n_nmi; 


void HardFault_Handler() 
{
  get_shared_memory()->crash_reason = CRASH_HARDFAULT; 
  reset(10); 
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

  /* Initialize io system */ 
  io_init(); 

  /* Initialize UART */ 
  i2c_bus_init(); 

  //persist previous state
  get_gpio_expander_state(0,0); 

  /** Initial state: SBC on (for now...) */ 
  i2c_gpio_expander_t turn_on_sbc = {.sbc=1}; 
  set_gpio_expander_state (turn_on_sbc,turn_on_sbc); 

  sbc_init(); 

  /* Initialize SPI flash */ 
  spi_flash_init(); 


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
    get_shared_memory()->crash_reason = CRASH_UNSET; 
  }

  //enable the calendar
  calendar_enable(&CALENDAR); 
  uint64_t time_check = 0; 

  /** Initialize LoRaWAN */ 
  lorawan_init(1); 


  /** Initialize ADC monitors */ 

  monitor_init(); 
   
  /** Initialize power system monitors */ 
  power_monitor_init(); 




  // TODO: setup watchdog

  if (ENABLE_WATCHDOG) 
  {
    wdt_enable(&INTERNAL_WATCHDOG); 
  } 

  //figure out current mode
  mode_init(); 


  /** The main control loop */ 

	while (1)
  {
    if (!low_power_mode) 
    {

      ///Start with potential inputs 

      //Check if we got an interrupt 
      if (n_interrupts > last_nint) 
      {
        printf("#INFO: number of interrupts now %d\r\n", ++last_nint); 
      }


      // Service any messages from the SBC
      sbc_io_process();

      // Service LTE (this does nothing for now) 
      lte_process(); 
     
      if (LTE_TURNON_NTICKS > 0  && nticks == LTE_TURNON_NTICKS) 
      {
        lte_turn_on(0); 
      }

      /// See if we need to do anything
      switch (nticks & 0x1fff)
      {
        case 0: 
          if (nticks > 0)
          {
            power_monitor_schedule(); 
          }
          monitor_fill(&last_mon,20); 
         break; 
        case 300: 
          power_monitor_fill(&last_power); 
        default: 
          break; 
      }
    }



    // Service LoRaWAN 
    lorawan_process(); 

    //
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
    


    /// See if we need to send anything 

    if (lorawan_state() == LORAWAN_READY) 
    {
      //our time isn't valid, let's request it
      if (nticks >= time_check ) 
      {
       int have_time = get_time() > 1000000000; 
       lorawan_request_datetime() ;
       int delay_in_secs = have_time ? 3600*4 : 15; 
       time_check+= (delay_in_secs*1000 / DELAY_MS) ;
      }

      //Let's testing sending something 
      if ((nticks & 0x2fff) == 0 && lorawan_state() == LORAWAN_READY) 
      {
        lorawan_tx_copy(sizeof(nticks), 2, (uint8_t*) &nticks,0); 
      }

    }

   
    if (ENABLE_WATCHDOG) 
    {
      wdt_feed(&INTERNAL_WATCHDOG); 
    }

    if (nticks==30) 
    {
      /** Reset reset counter if we made it through the loop 30 times*/ 
      get_shared_memory()->nresets = 0; 
    }


    if (low_power_mode && (wokeup_ticks - nticks) >= LOW_POWER_AWAKE_TICKS + extra_awake_ticks) 
    {
      //send report

      wokeup_ticks = ++nticks; 
      extra_awake_ticks = 0; 
      low_power_sleep_for_a_while(LOW_POWER_SLEEP_AMOUNT); 

      //after we wake up, we'll be here 
      power_monitor_schedule(); 
    }
    else
    {
      delay_ms(DELAY_MS); 
      nticks++; 
    }
	}
}
