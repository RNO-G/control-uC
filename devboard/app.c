#include <atmel_start.h> 

#include <string.h> 
#include <hal_io.h> 
#include <hal_delay.h>
#include "shared/sbc_uart.h" 
#include "shared/config_block.h"
#include "shared/spi_flash.h"

#include <stdio.h>

void print_block( config_block_t * b)
{

  printf("%d. %d %d %d %d\r\n", b->boot_cfg.n_resets_before_reflash, 
                                 b->boot_cfg.recovery_priority_list[0],
                                 b->boot_cfg.recovery_priority_list[1],
                                 b->boot_cfg.recovery_priority_list[2],
                                 b->boot_cfg.recovery_priority_list[3]);
}

int main()
{
	atmel_start_init();
  spi_flash_init(); 
  sbc_uart_init(); 


  const char * hello = "hello world\r\n"; 
  uint32_t device_id = spi_flash_device_id();
  char buf[256]; 
  printf("devid:0x%x\r\n",device_id);

  config_block_t block0; 
  config_block_t block1 = {0}; 
  default_init_block(&block0); 
  print_block(&block0);
  spi_flash_write_config_block(&block0); 
  spi_flash_read_config_block(&block1); 
  print_block(&block1);

  volatile int i = 0; 
  int level = 1;

  while(1)
  {
    printf("%d\r\n",i++);
    gpio_set_pin_level(LED,level); 
    level = !level; 
    delay_ms(100*(i++)); 
  }


}
