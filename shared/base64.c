
#include "shared/base64.h" 
#ifndef _HOST_
#include "shared/printf.h" 
#else 
#include <string.h> 
#endif 


static char val2char(uint8_t val) 
{
  if (val < 26) return val + 'A'; 
  else if (val < 52) return val-26 + 'a'; 
  else if (val < 62) return val-52 + '0'; 
  else return val == 62 ? '+' : 
              val == 63 ? '/' : 
              '!'; 
}

static uint8_t char2val(char c) 
{
  if ( c >= 'A' && c <= 'Z') return c-'A'; 
  else if ( c >= 'a' && c <= 'z') return c-'a'+26; 
  else if (c >='0' && c <='9') return c-'0' + 52; 
  else return c == '+' ? 62 : 
              c == '/' ? 63 : 64; 
}



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
    //no validation... 
    uint32_t partial =  (char2val(buf[i++])) & 0x3f;  

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

#ifdef _HOST_ 
int base64_fprintf(FILE *f, int len,  const uint8_t * buf)
#else
int base64_print(int d, int len,  const uint8_t * buf)
#endif
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
      output[0] = val2char(encoded >> 18);
      output[1] = val2char((encoded >> 12) & 0x3f) ;
      output[2] = j ==1 ? '=' : val2char(((encoded >> 6) & 0x3f));
      output[3] = j <=2 ? '=' : val2char(encoded & 0x3f);
#ifdef _HOST_ 
      ret += fprintf(f,"%c%c%c%c", output[0], output[1], output[2], output[3]); 
#else
      ret += dprintf(d, "%c%c%c%c", output[0], output[1], output[2], output[3]); 
#endif
      j=0; 
    }
  }
  return ret; 
}

#ifdef _TEST_ 

int main(int nargs, char ** args) 
{
  int decode = 0; 
  for (int i= 1 ; i < nargs; i++) 
  {
    if (!strcmp(args[i],"-d"))
    {
      printf("Decoding binary!\n"); 
      decode = 1 ; 
    }
    else if (!strcmp(args[i],"-c"))
    {
      printf("Decoding char!\n"); 
      decode = 2 ; 
    }
    else
    {
      FILE * f = fopen(args[i],"r"); 

      printf("%s:\n", args[i]); 
      char buf[48]; 
      while (!feof(f))
      {
        int N = fread(buf,1,sizeof(buf), f); 
        if (decode) 
        {
          int nb = base64_decode(N, buf); 
          for (int i = 0; i < nb; i++) 
          {
            printf(decode == 1 ? "[%x]": "%c", buf[i]); 
          }
        }
        else
        {
          base64_fprintf(stdout,N, buf); 
        }
    //    printf("\n"); 
      }
      printf("\n"); 
      fclose(f); 
    }
  }

  return 0; 
}


#endif
