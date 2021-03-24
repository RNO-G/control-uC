
#include "shared/base64.h" 
#include "shared/printf.h" 



int base64_decode(int b64_len, uint8_t * buf) 
{
  //read in groups of 4 

  int i = 0; 
  int j = 0; 
  int k = 0; 
  uint32_t decoded; 

  while (i < b64_len) 
  {
    if (buf[i] == '=') break; 
    if (j ==0) decoded = 0;

    uint32_t partial =  (buf[i++] -'A') & 0x3f;  

    decoded |= (partial << (6*(3-j))); 
    j++; 
    if (j == 4) 
    {
      buf[k++] = decoded >> 16; 
      buf[k++] = (decoded >> 8) & 0xff; 
      buf[k++] = (decoded ) & 0xff; 
      j=0;
    }

  }

  return k; 
}

int base64_print(int d, int len,  const uint8_t * buf)
{
  int i = 0; 
  int j = 0; 
  int ret = 0;
  uint32_t encoded; 
  while (i < len) 
  {
    if (j == 0) encoded = 0; 
    encoded |= (buf[i++] << (8*(2-j)));
    j++; 
    if (j == 3 || i == len) 
    {
      char output[4]; 
      output[0] = (encoded >> 18) + 'A' ;
      output[1] = ((encoded >> 12) & 0x3f) + 'A' ;
      output[2] = j ==1 ? '=' : ((encoded >> 6) & 0x3f) + 'A' ;
      output[3] = j ==2 ? '=' : (encoded & 0x3f) + 'A' ;
      ret += dprintf(d, "%c%c%c%c", output[0], output[1], output[2], output[3]); 
      j=0; 
    }
  }
  return ret; 
}
