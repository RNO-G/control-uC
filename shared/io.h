#ifndef _rno_g_io_h
#define _rno_g_io_h

#include <string.h> 


/** 
 *  IO helper functions 
 *
  */ 

//this match the same descripters as dprintf, can't use zero here! 

#define NONE_DESC 0
#define SBC_UART_DESC 1
#define SBC_UART_CONSOLE_DESC 2
#define N_DESC 3

void io_init(); 

int d_write(int d, int n, const uint8_t * data); 
int d_read(int d, int n, uint8_t * data); 
int d_write_ready(int d); 
int d_bytes_available(int d); 

static inline int d_put(int d, const char * str) { return d_write(d, strlen(str), (uint8_t*)  str); } 

static inline int sbc_uart_write_ready()  { return d_write_ready(SBC_UART_DESC); }
static inline int sbc_uart_put(const char *str) { return d_put(SBC_UART_DESC, str) ; }
static inline int sbc_uart_write(int n, uint8_t *data) { return d_write(SBC_UART_DESC,n,data) ; } 
static inline int sbc_uart_read(int n, uint8_t* data)  { return d_read(SBC_UART_DESC,n,data) ; } 
static inline int sbc_uart_bytes_available() { return d_bytes_available(SBC_UART_DESC); }

static inline int sbc_uart_console_write_ready() { return d_write_ready(SBC_UART_CONSOLE_DESC) ; }
static inline int sbc_uart_console_put(const char * str) { return d_put(SBC_UART_CONSOLE_DESC, str) ; } 
static inline int sbc_uart_console_write(int n, uint8_t* data) { return d_write(SBC_UART_CONSOLE_DESC, n, data) ; }
static inline int sbc_uart_console_read(int n, uint8_t*data) { return d_read(SBC_UART_CONSOLE_DESC, n, data) ; }
static inline int sbc_uart_console_bytes_available() { return d_bytes_available(SBC_UART_CONSOLE_DESC) ; }



#endif 

