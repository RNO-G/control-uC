
#include "shared/driver_init.h" 
#include "hpl_usart_async.h" 
#include "hal_usart_async.h" 
#include "shared/printf.h" 

#include <string.h>
#include "shared/io.h" 
#include <hal_io.h> 




static struct usart_async_descriptor * desc[4];  //enough for UARTs

static volatile uint64_t cb_called[4];  
static volatile uint64_t err_called[4];  
static volatile uint64_t last_err_called[4];  
static volatile uint8_t tx_in_progress[4];  

static const char * get_name(int d) 
{
  switch(d) 
  {
    case SBC_UART_DESC: return "SBC"; 
    case SBC_UART_CONSOLE_DESC: return "SBC_CONSOLE"; 
    case LTE_UART_DESC: return "LTE"; 
    default: return "UKNOWN"; 
  }
}



static void do_tokenized_read(async_tokenized_buffer_t * b) 
{
  if (!b) return;
  if (b->token_matched) return; 

  int d = b->desc; 
  struct io_descriptor * io = &desc[d]->io ; 
  const char * to_match = b->token ?: ""; 

  d_check(d,10); 
  while(usart_async_is_rx_not_empty(desc[d]) && ! b->token_matched)  
  {
    if (b->len >= b->capacity-1) 
    {
      b->noverflow++; 
      b->len =0; //kill the buffer 
      break; 
    }
    else
    {
      b->len += io_read(io, (uint8_t*)b->buf+b->len, 1); 

      int match =  (b->buf[b->len-1] == to_match[b->matched_chars]); 
      if (!match) 
      {
        b->matched_chars = 0; 
        if (b->drop_nulls && !b->buf[b->len-1])
        {
          b->len = 0; 
        }
      }
      else
      {
        b->matched_chars++; 
        if (!to_match[b->matched_chars-1] || !to_match[b->matched_chars])//handle the empty string gracefully 
        {
          b->token_matched = 1; 
          //null terminate at the start of the tken 
          b->buf[b->len-b->matched_chars] = 0; 
          break; 
        }
      }
    }
    b->nread_called++; 
  }
}

int async_tokenized_buffer_ready(async_tokenized_buffer_t *b)
{
  if (b->token_matched) return 1; 
  do_tokenized_read(b); 
  return b->token_matched; 
}

void async_tokenized_buffer_discard(async_tokenized_buffer_t * b) 
{
  b->matched_chars = 0; 
  b->len =0; 
  b->buf[0] = 0; 
  b->token_matched = 0; 
  do_tokenized_read(b); //read more! 
}


static void cb_sbc(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  cb_called[SBC_UART_DESC]++; 
}

static void cb_sbc_tx(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  tx_in_progress[SBC_UART_DESC]=0; 
}


static void cb_sbc_console(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  cb_called[SBC_UART_CONSOLE_DESC]++; 
}

static void cb_sbc_console_tx(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  tx_in_progress[SBC_UART_CONSOLE_DESC]=0; 
}


static void cb_lte(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  cb_called[LTE_UART_DESC]++; 
}

static void cb_lte_tx(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  tx_in_progress[LTE_UART_DESC]=0; 
}


static void cb_sbc_err(const struct usart_async_descriptor * dev) 
{
  (void) dev;
  err_called[SBC_UART_DESC]++; 
}

static void cb_sbc_console_err(const struct usart_async_descriptor * dev) 
{
   (void) dev;
   err_called[SBC_UART_CONSOLE_DESC]++; 
}

static void cb_lte_err(const struct usart_async_descriptor * dev) 
{
   (void) dev; 
   err_called[LTE_UART_DESC]++; 
}



void io_init()
{
  usart_async_enable(&SBC_UART); 
  desc[SBC_UART_DESC] = &SBC_UART; 
  usart_async_register_callback(&SBC_UART, USART_ASYNC_RXC_CB, cb_sbc); 
  usart_async_register_callback(&SBC_UART, USART_ASYNC_TXC_CB, cb_sbc_tx); 
  usart_async_register_callback(&SBC_UART, USART_ASYNC_ERROR_CB, cb_sbc_err); 
#ifndef _BOOTLOADER_
  usart_async_enable(&SBC_UART_CONSOLE); 
  desc[SBC_UART_CONSOLE_DESC] = &SBC_UART_CONSOLE; 
  usart_async_register_callback(&SBC_UART_CONSOLE, USART_ASYNC_RXC_CB, cb_sbc_console); 
  usart_async_register_callback(&SBC_UART_CONSOLE, USART_ASYNC_TXC_CB, cb_sbc_console_tx); 
  usart_async_register_callback(&SBC_UART_CONSOLE, USART_ASYNC_ERROR_CB, cb_sbc_console_err); 
  usart_async_enable(&LTE_UART); 
  desc[LTE_UART_DESC] = &LTE_UART; 
  usart_async_register_callback(&LTE_UART, USART_ASYNC_RXC_CB, cb_lte); 
  usart_async_register_callback(&LTE_UART, USART_ASYNC_TXC_CB, cb_lte_tx); 
  usart_async_register_callback(&LTE_UART, USART_ASYNC_ERROR_CB, cb_lte_err); 
#endif

}

int d_check(int d, int thresh) 
{
  int nerr = err_called[d] - last_err_called[d]; 
  if ( nerr > thresh) 
  {
     usart_async_disable(desc[d]); 
     usart_async_flush_rx_buffer(desc[d]); 
     usart_async_enable(desc[d]); 
     last_err_called[d]=err_called[d]; 
     tx_in_progress[d] = 0; 
     printf("#INFO: %d errors detected on %s\n", nerr, get_name(d)); 
     return 1; 
  }
  return 0; 
}




int d_write_ready(int d)
{
  return !tx_in_progress[d]; 
}

int d_write(int d, int n, const uint8_t * data) 
{
  volatile int ncycles = 0; 
  while (!d_write_ready(d)) //this should be enough
  {
    ; 
  }

  tx_in_progress[d] = 1; 
  int sent = io_write(&desc[d]->io, data, n); 
  return sent; 
}


int d_read(int d, int n, uint8_t * buf) 
{
   d_check(d, 10); 
   return usart_async_is_rx_not_empty(desc[d])
          ?  io_read(&desc[d]->io, buf,n) 
          : 0; 
}




// this buys us some time before the address is reused
static char putcharbuf[128]; 
static uint8_t putcharbuf_i = 0;
void _putchar(int d, char c)
{
  putcharbuf[putcharbuf_i] = c; 
  d_write(d, 1, (uint8_t*) putcharbuf+putcharbuf_i); 
  putcharbuf_i = (putcharbuf_i+1) % sizeof(putcharbuf); 
}


void flush_buffers() 
{
  usart_async_flush_rx_buffer(&SBC_UART); 
#ifndef _BOOTLOADER_
  usart_async_flush_rx_buffer(&SBC_UART_CONSOLE); 
  usart_async_flush_rx_buffer(&LTE_UART); 
#endif

}
