#ifndef _CMD_BUF_H
#define _CMD_BUF_H

#include <hal_io.h> 
#include "shared/io.h" 

/* Tokenizer with callback 
 *
 * Cosmin Deaconu <cozzyd@kicp.uchicago.edu>
 * */ 



/* Process command buffer inputs,calling the callback with every command. If callback returns one will stop processing. */
int buffer_tokenizer_process(async_read_buffer_t * buf, int token_length, uint8_t *token, int (*cb) (int len, uint8_t * data));



#endif
