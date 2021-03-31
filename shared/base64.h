#ifndef _rno_g_base_64_
#define _rno_g_base_64_

#include <stdint.h> 

#ifdef _HOST_
#include <stdio.h> 
#endif



/** in-place decode of base64 buffer, returns length of buffer */ 
int base64_decode(int b64_len, uint8_t *buf);


/** print base64 encoded data */ 
#ifdef _HOST_
int base64_fprintf(FILE* f, int raw_len, const uint8_t * buf); 
#else
int base64_print(int d, int raw_len, const uint8_t * buf); 
#endif




#endif
