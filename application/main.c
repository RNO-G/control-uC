#include "shared/driver_init.h" 
#include "lorawan/lorawan.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "application/sbc.h" 
#include "hal_ext_irq.h" 
#include "hal_gpio.h" 
#include "application/debug.h" 
#include "shared/spi_flash.h" 
#include "shared/shared_memory.h" 
#include "application/gpio_expander.h" 

#ifndef _DEVBOARD_
#include "hal_i2c_m_sync.h" 
#include "application/lte.h" 
#include "application/i2cbus.h" 
#endif 

/** This is still mostly a placeholder right now while testing!!! 
 *
 */ 





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

#ifndef _DEVBOARD_
  /* Initialize LTE UART */ 
  i2c_bus_init(); 
#endif

  sbc_init(); 


  /* Initialize SPI flash */ 
  spi_flash_init(); 



#ifndef _DEVBOARD_ 
#ifndef USE_RADIO_DEBUG
  ext_irq_register(GPIO1, interrupt); 
#endif
  lte_init(); 
#endif 

  // read in the config block
  config_block_t * cfg = config_block(); 


#ifndef _DEVBOARD_
  i2c_gpio_expander_t turn_on_sbc = {.sbc=1}; 
  set_gpio_expander_state (turn_on_sbc,turn_on_sbc); 

#endif

  get_shared_memory()->nresets = 0; 


  printf("IN APPLICATION\r\n"); 


  lorawan_init(); 
//
  int last_nint = 0; 


	while (1) {
    if (n_interrupts > last_nint) 
    {
      printf("number of interrupts now %d\n", ++last_nint); 
    }

    lorawan_process(); 
    if (sbc_io_process())
    {
      sbc_turn_on(0); 
    }

    delay_ms(10); 
    nticks++; 
	}
}
