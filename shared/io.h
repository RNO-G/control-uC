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

/** This will collect (via callbacks) characters from the UART 
 * until a token is reached. Guaranteed to be NULL terminated  */ 

typedef struct async_tokenized_buffer
{
  volatile uint8_t * buf; 
  const char * token;  // The token to match on (null-terminated). if empty string or NULL, match on null character, otherwise it's a null-terminated string. 
  volatile uint16_t len;  //max is capacity-1 due to null-termination 
  uint16_t capacity; 
  volatile uint8_t token_matched; // Did we match the token? 
  volatile uint8_t matched_chars; //how many chars have we matched
  uint8_t drop_nulls;  //if a null is encountered before the token, start over. 
  int desc; 
  int noverflow; 
  int nread_called ;
} async_tokenized_buffer_t; 

#define ASYNC_TOKENIZED_BUFFER(N, NAME, TOKEN, DESC) \
uint8_t NAME##_buf_[N]; \
async_tokenized_buffer_t NAME = {.buf = NAME##_buf_, .capacity = N, .len = 0, .drop_nulls=1, .token=TOKEN, .desc = DESC} 


int d_read(int d, int len, uint8_t* buf);  //non-blocking read 


/** This reads into the buffer, returns 1 if we have matched */ 
int async_tokenized_buffer_ready(async_tokenized_buffer_t * b); 
/* get rid of what's in thh buffer */ 
void async_tokenized_buffer_discard(async_tokenized_buffer_t * b); 

/* Return the length of what's in the buffer (but only if the token matched */ 
inline uint16_t async_tokenized_buffer_len(async_tokenized_buffer_t * b) { return b->token_matched ? b->len : 0; } 


/** This checks if there have been more than threshold errors since the last 
 * time this was called, and resets the UART if so. 
 * */ 
int d_check(int d, int threshold); 

int d_write_ready(int d); 

static inline int d_put(int d, const char * str) { return d_write(d, strlen(str), (uint8_t*)  str); } 
void flush_buffers(); 

static inline int sbc_uart_write_ready()  { return d_write_ready(SBC_UART_DESC); }
static inline int sbc_uart_put(const char *str) { return d_put(SBC_UART_DESC, str) ; }
static inline int sbc_uart_write(int n, uint8_t *data) { return d_write(SBC_UART_DESC,n,data) ; } 
static inline int sbc_uart_read( int n, uint8_t * data)  { return d_read(SBC_UART_DESC,n,data) ; } 

#ifndef _DEVBOARD_
#ifndef _BOOTLOADER_
static inline int sbc_uart_console_write_ready() { return d_write_ready(SBC_UART_CONSOLE_DESC) ; }
static inline int sbc_uart_console_put(const char * str) { return d_put(SBC_UART_CONSOLE_DESC, str) ; } 
static inline int sbc_uart_console_write(int n, uint8_t* data) { return d_write(SBC_UART_CONSOLE_DESC, n, data) ; }
static inline int sbc_uart_console_read( int n, uint8_t * data)  { return d_read(SBC_UART_CONSOLE_DESC,n,data) ; } 

static inline int lte_uart_write_ready() { return d_write_ready(LTE_UART_DESC) ; }
static inline int lte_uart_put(const char * str) { return d_put(LTE_UART_DESC, str) ; } 
static inline int lte_uart_write(int n, uint8_t* data) { return d_write(LTE_UART_DESC, n, data) ; }
static inline int lte_uart_read( int n, uint8_t * data)  { return d_read(LTE_UART_DESC,n,data) ; } 
#endif
#endif



#endif 

