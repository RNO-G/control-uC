#ifndef _base_64_
#define _base_64_

#include <stdint.h> 



/** in-place decode of base64 buffer, returns length of buffer */ 
int base64_decode(int b64_len, uint8_t *buf);


/** print base64 encoded data */ 
int base64_print(int d, int raw_len, const uint8_t * buf); 




#endif
