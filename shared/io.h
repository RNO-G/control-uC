#ifndef _rno_g_io_h
#define _rno_g_io_h

#include "hal_atomic.h"

#include <stdint.h> 
#include <string.h> 

/** 
 *  IO helper functions 
 *
  */ 

//this match the same descripters as dprintf, can't use zero here! 

#define NONE_DESC 0
#define SBC_UART_DESC 1
#define SBC_UART_CONSOLE_DESC 2
#define LTE_UART_DESC 3
//#define USB_CDC_DESC  3

void io_init(); 

int d_write(int d, int n, const uint8_t * data); 

typedef struct async_read_buffer
{
  volatile uint8_t * buf; 
  uint16_t length; 
  volatile uint16_t offset; 
  volatile int busy; 
} async_read_buffer_t; 

#define ASYNC_READ_BUFFER(N, NAME) \
uint8_t NAME##_buf_[N]; \
async_read_buffer_t NAME = {.buf = NAME##_buf_, .length = N, .offset = 0, .busy = 0} 


//note that this is asynchronous! will return immediately and continue  reading either
//until n is hit or another call to d_read is made. 
int d_read_async(int d, async_read_buffer_t * buffer); 
async_read_buffer_t * get_read_buffer(int d); 

//shift the read buffer discarding the first N bytes and moving the rest of the bytes forward
//useful for e.g. parsing commands. This is very inefficient compared to a circular buffer
//but is more space efficient (Since you don't need tomake a copy of the data across the boundary). Another option would be
//a circular buffer with an overlap. 
void async_read_buffer_shift(async_read_buffer_t * b, int N); 
static inline void async_read_buffer_clear(async_read_buffer_t *b) { async_read_buffer_shift(b,b->offset); }
static inline uint8_t* async_read_buffer_seek(async_read_buffer_t *b, uint8_t * what, int what_len)  { return (uint8_t*) memmem((void*)b->buf, b->offset, what, what_len); }
static inline char* async_read_buffer_seek_str(async_read_buffer_t *b, char * what)  { return (char*) memmem((void*)b->buf, b->offset, (const char*) what, strlen(what)); }

/** if c is between buffer start and offset, shifts until first instance of character. Can be used to e.g. remove \0 or whatever. Returns number of chars shifted. */ 
int async_read_buffer_shift_until_char(async_read_buffer_t * b, uint8_t c)  ; 

int d_write_ready(int d); 

static inline int d_put(int d, const char * str) { return d_write(d, strlen(str), (uint8_t*)  str); } 

static inline int sbc_uart_write_ready()  { return d_write_ready(SBC_UART_DESC); }
static inline int sbc_uart_put(const char *str) { return d_put(SBC_UART_DESC, str) ; }
static inline int sbc_uart_write(int n, uint8_t *data) { return d_write(SBC_UART_DESC,n,data) ; } 
static inline int sbc_uart_read_async(async_read_buffer_t * b)  { return d_read_async(SBC_UART_DESC,b) ; } 

#ifndef _DEVBOARD_
#ifndef _BOOTLOADER_
static inline int sbc_uart_console_write_ready() { return d_write_ready(SBC_UART_CONSOLE_DESC) ; }
static inline int sbc_uart_console_put(const char * str) { return d_put(SBC_UART_CONSOLE_DESC, str) ; } 
static inline int sbc_uart_console_write(int n, uint8_t* data) { return d_write(SBC_UART_CONSOLE_DESC, n, data) ; }
static inline int sbc_uart_console_read_async(async_read_buffer_t * b) { return d_read_async(SBC_UART_CONSOLE_DESC,b) ; }

static inline int lte_uart_write_ready() { return d_write_ready(LTE_UART_DESC) ; }
static inline int lte_uart_put(const char * str) { return d_put(LTE_UART_DESC, str) ; } 
static inline int lte_uart_write(int n, uint8_t* data) { return d_write(LTE_UART_DESC, n, data) ; }
static inline int lte_uart_read_async(async_read_buffer_t * b) { return d_read_async(LTE_UART_DESC, b) ; }
#endif
#endif

/*
static inline int usb_cdc_console_write_ready() { return d_write_ready(USB_CDC_DESC) ; }
static inline int usb_cdc_console_put(const char * str) { return d_put(USB_CDC_DESC, str) ; } 
static inline int usb_cdc_console_write(int n, uint8_t* data) { return d_write(USB_CDC_DESC, n, data) ; }
static inline int usb_cdc_console_read(int n, uint8_t*data) { return d_read(USB_CDC_DESC, n, data) ; }
static inline int usb_cdc_console_bytes_available() { return d_bytes_available(USB_CDC_DESC) ; }
*/



#endif 

