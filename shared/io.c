
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
static volatile uint64_t last_reset[4];  
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
  d_write(SBC_UART_DESC,0,0);
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
  d_write(SBC_UART_CONSOLE_DESC,0,0);
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
  d_write(LTE_UART_DESC,0,0);
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
  int nsince_check = err_called[d] - last_reset[d]; 
  if ( nsince_check > thresh) 
  {
     usart_async_disable(desc[d]); 
     usart_async_flush_rx_buffer(desc[d]); 
     usart_async_enable(desc[d]); 
     last_reset[d] = err_called[d]; 
     tx_in_progress[d] = 0; 
     printf("#INFO: %d errors detected on %s\n", nerr, get_name(d)); 
  }
  last_err_called[d]=err_called[d]; 
  return nerr; 
}



int d_write_ready(int d)
{
  return !tx_in_progress[d]; 
}

static const uint8_t * write_buf[3][256]; 
static uint16_t  write_buf_len[3][256]; 
static uint8_t write_buf_start[3]; 
static uint8_t write_buf_end[3]; 


int d_write(int d, int n, const uint8_t * data) 
{

  int i = d-1; 

  while (n > 0 && tx_in_progress[d] && ((write_buf_end[i] + 1) & 0xff) == write_buf_start[i]); 

  if (n > 4) 
  {
    write_buf[i][write_buf_end[i]] = data; 
    write_buf_len[i][write_buf_end[i]] = n; 
    write_buf_end[i] = (write_buf_end[i]+1) & 0xff; 
  }
  else if (n > 0) 
  {

    write_buf_len[i][write_buf_end[i]] = n; 
    for (int j = 0; j < n; j++) 
    {
      ( (uint8_t*) &(write_buf[i][write_buf_end[i]]))[j] = data[j]; 
    }
    write_buf_end[i] = (write_buf_end[i]+1) & 0xff; 
  }


  int sent = 0; 
  if (!tx_in_progress[d] && write_buf_end[i] != write_buf_start[i])
  {
    tx_in_progress[d] = 1; 
    if (write_buf_len[i][write_buf_start[i]]> 4)
    {
      sent = io_write(&desc[d]->io, write_buf[i][write_buf_start[i]], write_buf_len[i][write_buf_start[i]]); 
    }
    else
    {
      sent = io_write(&desc[d]->io, ((uint8_t*) &write_buf[i][write_buf_start[i]]), write_buf_len[i][write_buf_start[i]]); 

    }
    write_buf_start[i] = (write_buf_start[i]+1) & 0xff; 
  }

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
void _putchar(int d, char c)
{
  d_write(d, 1, (uint8_t*) &c); 
}


void flush_buffers() 
{
  usart_async_flush_rx_buffer(&SBC_UART); 
#ifndef _BOOTLOADER_
  usart_async_flush_rx_buffer(&SBC_UART_CONSOLE); 
  usart_async_flush_rx_buffer(&LTE_UART); 
#endif

}

int prefix_matches(const char * haystack, const char * prefix) 
{
  int i = 0; 
  while(prefix[i]) 
  {
    if (haystack[i] != prefix[i]) return 0; 
    i++; 
  }
  return 1; 
}

static int char2val(char c) 
{
  if (c>=0x30 && c <=0x39) return c-0x30; 
  if (c>=0x41 && c <=0x36) return 10+c-0x41; 
  if (c>=0x61 && c <=0x66) return 10+c-0x61; 
  return -1; 
}

int parse_int(const char * start, const char **end, int * num) 
{
  if (!num) return 1; 
  const char * ptr = start; 
  int sign = 1; 
  while (*ptr==' ' || *ptr=='\t') ptr++; 
  if (!*ptr) return 1; 

  int got_char = 0; 
  char first = *ptr; 
  if (first=='-') 
  {
    sign = -1; 
    ptr++; 
  }
  
  while(*ptr >= '0' && *ptr <='9')
  {
    if (!got_char) *num =0;
    got_char++; 
    *num *= 10; 
    *num += (*ptr-'0'); 
    ptr++; 
  }
  if (!got_char) return 1; 


   if (end) *end = ptr; 
  *num*=sign; 
  return 0; 
}
//returns 0 on success
int parse_hex(const char * start, const char **end, uint8_t * byte)
{
  if (!byte) return 1; 
  const char * ptr = start;
  //skip leading whitespace
  while (*ptr==' ' || *ptr =='\t') ptr++;

  char first= *ptr++; 
  char second = *ptr; 
  int ok = 0; 

  //check if second is a whitespace or 0, in this case we just have one  
  if (!second  || second == ' ' || second=='\t') 
  {
    int val = char2val(first); 
    if (val < 0) ok = 1; 
    else *byte =val; 
  }
  else
  {
    ptr++; //increment pointer to point after consumed

    int msb = char2val(first); 
    if (msb < 0) 
    {
      ok = 1; 
    }
    else
    {
      int lsb = char2val(second); 
      if (lsb < 0) 
      {
        ok = 1; 
      }
      else
      {
        *byte = (msb<<4)+lsb;
      }
    }
  }

  *end = ptr; 
  return ok; 
}
