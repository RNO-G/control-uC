#ifdef _BOOTLOADER_
#include "bootloader/bootloader_driver_init.h" 
#else
#include "driver_init.h" 
#endif 

#include "shared/io.h" 
#include <hal_io.h> 
#include <hal_usart_async.h> 

#ifdef _DEVBOARD_
#define SBC_UART USART_0 
#endif 


static struct usart_async_descriptor * desc[N_DESC];  


void io_init()
{
  usart_async_enable(&SBC_UART); 
  desc[SBC_UART_DESC] = &SBC_UART; 
#ifndef _DEVBOARD_
#ifndef _BOOTLOADER_
  usart_async_enable(&SBC_UART_CONSOLE); 
  desc[SBC_UART_CONSOLE_DESC] = &SBC_UART_CONSOLE; 
#endif 
#endif
}


int d_bytes_available(int d) 
{
 return usart_async_is_rx_not_empty(desc[d]); 
}


int d_write_ready(int d)
{
  return desc[d]->tx_por == desc[d]->tx_buffer_length; 
}

int d_write(int d, int n, const uint8_t * data) 
{
  while (!d_write_ready(d))
  {
    ; 
  }

  int sent = io_write(&desc[d]->io, data, n); 
  return sent; 
}

int d_read(int d, int n, uint8_t * data) 
{
  if (!d_bytes_available(d)) return 0; 
  return io_read(&desc[d]->io, data, n); 
}

void _putchar(int d, char c)
{
  d_write(d, 1, (uint8_t*) &c); 
}

