#include "shared/driver_init.h" 
#include "lorawan/lorawan.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "application/sbc.h" 
#include "hal_ext_irq.h" 
#include "hal_gpio.h" 
#include "hal_calendar.h" 
#include "application/debug.h" 
#include "shared/spi_flash.h" 
#include "shared/shared_memory.h" 
#include "application/gpio_expander.h" 

#include "hal_i2c_m_sync.h" 
#include "application/lte.h" 
#include "application/i2cbus.h" 

/** This is still mostly a placeholder right now while testing!!! 
 *
 */ 





static int last_nint = 0; 
static volatile int n_interrupts; 
static volatile int n_nmi; 


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

int main(void)
{
	/* Initialize system drivers*/ 
	system_init();

  /* Initialize io system */ 
  io_init(); 

  /* Initialize LTE UART */ 
  i2c_bus_init(); 

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


  /** Initial state: SBC on (for now...) */ 
  i2c_gpio_expander_t turn_on_sbc = {.sbc=1}; 
  set_gpio_expander_state (turn_on_sbc,turn_on_sbc); 

  printf("#INFO: BOOTED! \r\n"); 



  /** Initialize LoRaWAN */ 
  lorawan_init(); 




  /** Reset reset counter */ 
  get_shared_memory()->nresets = 0; 

  // TODO: setup watchdog

  // TODO: initial measurements



  /** The main control loop */ 

	while (1) {

    ///Start with potential inputs 

    //Check if we got an interrupt 
    if (n_interrupts > last_nint) 
    {
      printf("#INFO: number of interrupts now %d\r\n", ++last_nint); 
    }

    // Service any messages from the SBC
    if (sbc_io_process())
    {
      sbc_turn_on(0); 
    }

    // Service LTE
    lte_process(); 
   
    // Service LoRaWAN 
    lorawan_process(); 



    /// See if we need to do anything




    /// See if we need to send anything 

    // Do we have time? 
    // If not we should ask for it



    //Let's testing sending something 
    if ((nticks & 0x7ff) == 0 && lorawan_state() == LORAWAN_READY) 
    {
      lorawan_tx_copy(sizeof(nticks), 2, (uint8_t*) &nticks,0); 



    }
    


    delay_ms(10); 
    nticks++; 
	}
}
