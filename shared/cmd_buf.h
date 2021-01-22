#ifndef _CMD_BUF_H
#define _CMD_BUF_H

#include <hal_io.h> 

/* Command buffer for tokenizing of input commands 
 *
 * Cosmin Deaconu <cozzyd@kicp.uchicago.edu>
 * */ 


typedef struct cmd_buf
{
  int N; 
  char *buf;
  char token; 
  struct io_descriptor * io; 
  int offset;
  int (*cb) ( const char * cmd, void * data ); 
  void * data; 
} cmd_buf_t; 


/** Initialize command buffer, Token is '\n' by default but you can change afterwards. 
 * Note that N should be at least a bit more than twice as long as the longest command you want to reliably support. 
 * The callback should return 1 if you want to break out of the process loop early
 **/ 
void cmd_buf_init(cmd_buf_t * cmd_buf, 
                  int N, char * buf,
                  struct io_descriptor * io
                  int (*cb) (const char * cmd, void * data)
                  void * data, 
                  ); 

/* Process command buffer inputs,calling the callback with every command */
int cmd_buf_process(cmd_buf_t * cmd_buf); 



#endif
