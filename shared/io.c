
#include "shared/driver_init.h" 
#include "hpl_usart_async.h" 
#include "hal_usart_async.h" 

#include "shared/io.h" 
#include <hal_io.h> 



struct async_iod
{
  int ncalled; 
  async_read_buffer_t * b; 
}; 


static struct async_iod iods[4]; 
static struct usart_async_descriptor * desc[4];  //enough for UARTs

async_read_buffer_t * get_read_buffer(int d) { return iods[d].b; } 


static void callback_rx(const struct usart_async_descriptor * const dev, int d)
{

  iods[d].ncalled++; 

  async_read_buffer_t * b = get_read_buffer(d); 
  if (!b) return;

  while (usart_async_is_rx_not_empty(desc[d]))
  {
    if (b->offset >= b->length) 
    {
      return; 
    }
    b->busy = 1; 
    b->offset+= io_read(&desc[d]->io, b->buf + b->offset, b->length - b->offset); 
  }
  b->busy = 0; 
}


static void callback_rx_sbc_uart(const struct usart_async_descriptor * const dev) 
{
  callback_rx(dev,SBC_UART_DESC);
}

#ifndef _BOOTLOADER_
#ifndef _DEVBOARD_
static void callback_rx_sbc_uart_console(const struct usart_async_descriptor * const dev) 
{
  callback_rx(dev,SBC_UART_CONSOLE_DESC);
}

static void callback_rx_lte_uart(const struct usart_async_descriptor * const dev) 
{
  callback_rx(dev,LTE_UART_DESC);
}

#endif
#endif


void io_init()
{
  usart_async_enable(&SBC_UART); 
  desc[SBC_UART_DESC] = &SBC_UART; 
  usart_async_register_callback(&SBC_UART, USART_ASYNC_RXC_CB, &callback_rx_sbc_uart);
#ifndef _BOOTLOADER_
#ifndef _DEVBOARD_
  usart_async_enable(&SBC_UART_CONSOLE); 
  desc[SBC_UART_CONSOLE_DESC] = &SBC_UART_CONSOLE; 
  usart_async_register_callback(&SBC_UART_CONSOLE, USART_ASYNC_RXC_CB, &callback_rx_sbc_uart_console);

  usart_async_enable(&LTE_UART); 
  desc[LTE_UART_DESC] = &LTE_UART; 
  usart_async_register_callback(&LTE_UART, USART_ASYNC_RXC_CB, &callback_rx_lte_uart);
#endif
#endif

}



int d_write_ready(int d)
{
	return _usart_async_is_byte_sent(&desc[d]->device);
}

int d_write(int d, int n, const uint8_t * data) 
{
  volatile int ncycles = 0; 
  while (!d_write_ready(d) && ncycles++ < 480000) //this should be enough
  {
    ; 
  }

  int sent = io_write(&desc[d]->io, data, n); 
  return sent; 
}
int d_read_async(int d, async_read_buffer_t * b) 
{
  CRITICAL_SECTION_ENTER();
  iods[d].b = b; 
  CRITICAL_SECTION_LEAVE(); 
  return 0;
}


void async_read_buffer_shift(async_read_buffer_t * b, int N) 
{
  CRITICAL_SECTION_ENTER();
  //wait if there's an ongoing write going
  while (b->busy); 

  // just invalidate the whole thing
  if (N >= b->offset)
  {
    b->offset = 0; 
  }
  else //move the back to the front 
  {
    memmove(b->buf, b->buf+N, b->offset-N); 
    b->offset -= N; 

  }
  CRITICAL_SECTION_LEAVE();
}


// this buys us some time before the address is reused
static char putcharbuf[64]; 
static uint8_t putcharbuf_i = 0;
void _putchar(int d, char c)
{
  putcharbuf[putcharbuf_i] = c; 
  d_write(d, 1, (uint8_t*) putcharbuf+putcharbuf_i); 
  putcharbuf_i = (putcharbuf_i+1) % sizeof(putcharbuf); 
}

