#include "application/driver_init.h" 
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


ASYNC_READ_BUFFER(256, sbc); 

#ifndef _DEVBOARD_
ASYNC_READ_BUFFER(512, sbc_console); 
ASYNC_READ_BUFFER(256, lte); 
#endif

static  config_block_t cfg; 
static char lte_buf[] = "\r\nAT+GMR\r\n"; 


static volatile int n_interrupts; 


static void interrupt()
{
  n_interrupts++; 
}

int main(void)
{
	/* Initialize system drivesr*/ 
	system_init();
  /* Initialize io system */ 
  io_init(); 
  spi_flash_init(); 


  sbc_uart_read_async(&sbc); 


#ifndef _DEVBOARD_
  sbc_uart_console_read_async(&sbc_console); 
  lte_uart_read_async(&lte); 
  /* Initialize i2c bus */ 
  i2c_bus_init(); 
  ext_irq_register(GPIO1, interrupt); 
#endif 

  spi_flash_read_config_block(&cfg); 

  //to exercise 
  if (cfg.app_cfg.station_number != 123) 
  {
    cfg.app_cfg.station_number = 123; 
    spi_flash_write_config_block(&cfg); 
  }

#ifndef _DEVBOARD_
  //turn on the computer
  i2c_task_t task1 = {.addr = 0x3a, .write = 1, .reg = I2C_EXPANDER_SET_REGISTER, .data = 0x10, .done = 0}; 
  i2c_task_t task2 = {.addr = 0x3a, .write = 1, .reg = I2C_EXPANDER_CONFIGURE_REGISTER, .data = ~(0x10), .done = 0};
  i2c_enqueue(&task1); 
  i2c_enqueue(&task2); 

  i2c_task_t task3 = {.addr = 0x3f, .write = 0, .reg = I2C_EXPANDER_GET_REGISTER, .data = 0xab, .done = 0}; 
  i2c_enqueue(&task3); 
  delay_ms(1);
  i2c_task_t task4 = {.addr = 0x3a, .write = 0, .reg = I2C_EXPANDER_CONFIGURE_REGISTER, .data = 0x0, .done = 0}; 
  i2c_enqueue(&task4); 
#endif

  get_shared_memory()->nresets = 0; 


  
  lorawan_init(); 


  printf("IN APPLICATION"); 

  int last_nint = 0; 


	while (1) {
    if (n_interrupts > last_nint) 
    {
      printf("number of interrupts now %d\n", ++last_nint); 
    }

    lorawan_process(); 


#ifndef _DEVBOARD_
    char * where = strstr((char*)sbc.buf, "#LTE-ON\r\n");
    if (where)
    {
      *where=0; 
      async_read_buffer_shift(&sbc, 512); 
      lte_turn_on(); 
      printf("Turning on LTE\n"); 
    }



    where = strstr((char*)sbc.buf, "#LTE-OFF\r\n");
    if (where)
    {
      *where=0; 
      async_read_buffer_shift(&sbc, 512); 
      lte_turn_off(); 
      printf("Turning off LTE\n"); 
    }
#endif

	}
}
