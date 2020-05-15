#include "shared/driver_init.h" 
#include "lorawan/lorawan.h" 
#include "shared/io.h" 
#include "shared/printf.h" 
#include "hal_ext_irq.h" 
#include "hal_gpio.h" 
#include "application/debug.h" 
#include "shared/spi_flash.h" 
#include "shared/shared_memory.h" 

#ifndef _DEVBOARD_
#include "hal_i2c_m_sync.h" 
#include "application/lte.h" 
#include "application/i2cbus.h" 
#endif 

/** This is still mostly a placeholder right now while testing!!! 
 *
 */ 

ASYNC_READ_BUFFER(256, sbc); 

#ifndef _DEVBOARD_
ASYNC_READ_BUFFER(512, sbc_console); 
#endif

static  config_block_t cfg; 
static char lte_buf[] = "\r\nAT+GMR\r\n"; 


static volatile int n_interrupts; 
static volatile int n_nmi; 



// bookkeeping for i2c expander output state 
// TODO: make a saner interface in i2cbus
// these both default to 0xff! 
static i2c_task_t i2c_expander_a_state = {.addr = I2C_EXPANDER_A,
                                          .write = 1, 
                                          .reg=I2C_EXPANDER_SET_REGISTER, 
                                          .data = 0xff, 
                                          .done = 0 };

static i2c_task_t i2c_expander_a_dir = { .addr = I2C_EXPANDER_A, 
                                         .write = 1,
                                         .reg=I2C_EXPANDER_CONFIGURE_REGISTER,
                                         .data = 0xff,
                                         .done = 0 }; 
#define RADIANT_ENABLE_BIT I2C_EXPANDER_5V_ENABLE_1_BIT

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

  /* Initialize SPI flash */ 
  spi_flash_init(); 

  /* Initialize main SBC UART */ 
  sbc_uart_read_async(&sbc); 


#ifndef _DEVBOARD_
  /* Initialize SBC Console UART */ 
  sbc_uart_console_read_async(&sbc_console); 

  /* Initialize LTE UART */ 
  i2c_bus_init(); 
  ext_irq_register(GPIO1, interrupt); 
  lte_init(); 
#endif 

  spi_flash_read_config_block(&cfg); 

  //to exercise 
  if (cfg.app_cfg.station_number != 123) 
  {
    cfg.app_cfg.station_number = 123; 
    spi_flash_write_config_block(&cfg); 
  }

#ifndef _DEVBOARD_
  //turn on the sbc for now, nothing else. 
  i2c_expander_a_state.data = (1 << I2C_EXPANDER_SBC_ENABLE_BIT);

  // setup the outputs on exander a as outputs (note the inversion) 
  i2c_expander_a_dir.data = ~ ( (1 << I2C_EXPANDER_SBC_ENABLE_BIT) | (1 << RADIANT_ENABLE_BIT)); 

  i2c_enqueue(&i2c_expander_a_state); 
  i2c_enqueue(&i2c_expander_a_dir); 

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


#ifndef _DEVBOARD_

    //TEMPORARY REALLY DUMB COMMANDS 
    //
    char * where = strstr((char*)sbc.buf, "#LTE-ON\r\n");
    if (where)
    {
      *where=0; 
      async_read_buffer_shift(&sbc, 512); 
      lte_turn_on(); 
      printf("Turning on LTE\\rn"); 
    }



    where = strstr((char*)sbc.buf, "#LTE-OFF\r\n");
    if (where)
    {
      *where=0; 
      async_read_buffer_shift(&sbc, 512); 
      lte_turn_off(); 
      printf("Turning off LTE\r\n"); 
    }

    where = strstr((char*)sbc.buf,"#RADIANT-ON\r\n"); 
    if (where) 
    {
      *where = 0; 
      async_read_buffer_shift(&sbc,512); 
      i2c_expander_a_state.data |= (1 << RADIANT_ENABLE_BIT);
      i2c_enqueue(&i2c_expander_a_state); 
      printf("Turning on RADIANT\r\n"); 
    }

    where = strstr((char*)sbc.buf,"#RADIANT-OFF\r\n"); 
    if (where) 
    {
      *where = 0; 
      async_read_buffer_shift(&sbc,512); 
      i2c_expander_a_state.data &= ~(1 << RADIANT_ENABLE_BIT);
      i2c_enqueue(&i2c_expander_a_state); 
      printf("Turning on RADIANT\r\n"); 
    }
#endif

    delay_ms(10); 
    nticks++; 

	}
}
