#include <stdio.h> 
#include <stdlib.h> 
#include "shared/base64.h" 
#include <string.h>

int slot = 0; 
const char * device = "/dev/ttyO1" ;
const char * image = 0 ; 
FILE * fserial; 
int offset = 0; 
int nbytes = (256-16)*1024; 

void usage() 
{
  printf("Usage: rno-g-controller-extract [-h] [-s SLOT=1] [-d DEVICE=/dev/ttyO1] [-o OFFSET=0] [-N NBYTES=__ROM_SIZE__] image.bin\n"); 
  printf("   -h  Display this message\n");
  printf("   -s  SLOT  Pick the slot to extract from (default 1).  \n");
  printf("   -d  DEVICE The tty device (default /dev/ttyO1). Note that the device must already be configured appropriately, including converting \\n to \\r\\n on output. \n"); 
}

int prefix_matches(const char * haystack, const char * prefix) 
{
  int i = 0; 
  while(prefix[i] && haystack[i]) 
  {
    if (haystack[i] != prefix[i]) return 0; 
    i++; 
  }
  return 1; 
}

int parse_opts(int nargs, char ** args) 
{
  for (int iarg = 1; iarg < nargs; iarg++) 
  {
    if (!strcmp(args[iarg],"-h")) return 1; 
    else if (!strcmp(args[iarg],"-s"))
    {
      if (iarg++ == nargs) return 1; 
      slot = atoi(args[iarg]); 
    }
    else if (!strcmp(args[iarg],"-o"))
    {
      if (iarg++ == nargs) return 1; 
      offset = atoi(args[iarg]); 
    }
    else if (!strcmp(args[iarg],"-N"))
    {
      if (iarg++ == nargs) return 1; 
      nbytes = atoi(args[iarg]); 
    }
    else if (!strcmp(args[iarg],"-d"))
    {
      if (iarg++ == nargs) return 1; 
      device = args[iarg]; 
    }
    else if (image) 
    {
      return 1; 
    }
    else image = args[iarg]; 
  }
  return 0; 
}

char line[512]; 
const char* send_acked_command (const char * cmd, const char * args)
{
  fprintf(fserial,"%s %s\r\n",cmd,args); 

  const int max_attempts_for_ack = 3; 
  int nattempts = 0; 
  while (nattempts++ < max_attempts_for_ack) 
  {
    fgets(line, sizeof(line), fserial); 
    if (prefix_matches(line,cmd))
    {
      return line; 
    }
    else if (prefix_matches(line,"#ERR"))
    {
      fprintf(fserial,"%s %s\n",cmd,args); 
    }
  }
  return 0; 
}



int main(int nargs, char ** args) 
{
  if(!parse_opts(nargs, args) || !image )
  {
     usage(); 
     return 1; 
  }


  FILE * fimage = fopen(image,"w"); 
  if (!fimage) 
  {
    fprintf(stderr,"Could not open %s for writing\n", image);
    return 1; 
  }

  char cmdargs[128]; 
  for (int i = 0; i < nbytes; i+=128)
  {
    int ntoread = i + 128 > nbytes ? nbytes-i : 128; 
    snprintf(cmdargs,sizeof(cmdargs)," %d %d %d", slot, offset + i*128, ntoread); 
    const char * response = send_acked_command("#PROG-READ", cmdargs); 
    if (!response) 
    {
       fprintf(stderr,"read attempt failed %s\n", cmdargs); 
       fclose(fimage); 
       return 1; 
    }

    int slot_check, offset_check, len_check; 
    uint8_t base64buf[256]; 
    sscanf(response,"#PROG-READ(slot=%d,offset=%d,len=%d): %s", &slot_check, &offset_check, &len_check, (char*)base64buf) ; 
    if(slot!=slot_check || offset+i*128!=offset_check || ntoread!=len_check)
    {
       fprintf(stderr, "read arg mismatch %s vs. %s\n", cmdargs, response); 
       fclose(fimage); 
       return 1; 
    }

    int nb = base64_decode(strlen((char*)base64buf), base64buf); 
    if (nb!=fwrite(base64buf, 1,nb,fimage))
    {
      fprintf(stderr,"Write problem\n"); 
      fclose(fimage); 
      return 1; 
    }
  }

  fclose(fimage); 

  return 0;
}


