#include <stdio.h> 
#include <stdlib.h> 
#include "shared/base64.h" 
#include <string.h>
#include <errno.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/file.h> 

int slot = 1; 
const char * device = "/dev/ttyO1" ;
const char * image = 0 ; 
FILE * fserial = 0; 
int offset = 0; 
int nbytes = (256-16)*1024; 
int dump = 0;

void usage() 
{
  printf("Usage: rno-g-controller-extract [-h] [-x] [-s SLOT=1] [-d DEVICE=/dev/ttyO1] [-o OFFSET=0] [-N NBYTES=%d] image.bin\n", nbytes); 
  printf("   -h  Display this message\n");
  printf("   -s  SLOT  Pick the slot to extract from (default 1).  \n");
  printf("   -d  DEVICE The tty device (default /dev/ttyO1). Note that the device must already be configured appropriately, including converting \\n to \\r\\n on output. \n"); 
  printf("   -N  NBYTES Number of bytes to read (default %d). \n", nbytes); 
  printf("   -o  OFFSET Offset from start of slot(default %d). \n", offset); 
  printf("   -x  HEXDUMP to stdout . \n"); 
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
    else if (!strcmp(args[iarg],"-x"))
    {
	    dump=1;
    }
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
  const int max_attempts_for_ack = 20; 
  int nattempts = 0; 

  while (fgets(line,sizeof(line), fserial)); 
  usleep(1000); 
  while (nattempts < max_attempts_for_ack) 
  {
    if (nattempts % 5 == 0)
    {
      int written = fprintf(fserial,"%s %s\n",cmd,args); 
      if (written <=0) 
      {
        printf("written=%d, errno=%d, %s!\n", written, errno,strerror(errno)); 
	      return 0; 
      }

      usleep(10000); 
    }

    if (!fgets(line, sizeof(line), fserial))
    {
      usleep((nattempts+1)*5000); 
    }
    else if (prefix_matches(line,cmd))
    {
      return line; 
    }
    else if (prefix_matches(line,"#ERR"))
    {
      fprintf(stderr,"%s\n",line); 
      usleep(15000);
//      fprintf(fserial,"%s %s\n",cmd,args); 
//      usleep(5000);
//      nattempts=0; 
    }

    nattempts++; 


  }
  return 0; 
}



int main(int nargs, char ** args) 
{
  if(parse_opts(nargs, args) || !image )
  {
     usage(); 
     return 1; 
  }

  fserial = fopen(device,"a+"); 
  if (!fserial) 
  {
    fprintf(stderr,"Could not open %s for r/w\n", device); 
    return 2; 
  }


  int fd = fileno(fserial); 
  if (flock(fd,LOCK_EX))
  {
    fprintf(stderr,"Could not get exclusive access\n"); 
    return 3; 
  }

  int flags = fcntl(fd, F_GETFL,0); 
  flags |=O_NONBLOCK; 
  fcntl(fd, F_SETFL,flags); 



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
    snprintf(cmdargs,sizeof(cmdargs)," %d %d %d", slot, offset + i, ntoread); 
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
    if(slot!=slot_check || offset+i!=offset_check || ntoread!=len_check)
    {
       fprintf(stderr, "read arg mismatch %s vs. %s (%d %d %d)\n", cmdargs, response, slot_check, offset_check, len_check); 
       usleep(10000); 
       while(fgets(line, sizeof(line), fserial)); 
       i-=128; 
    }
    else
    {
      int nb = base64_decode(strlen((char*)base64buf), base64buf); 
      if (nb != ntoread) 
      {
        fprintf(stderr,"mismatch!!! %d %d\n" , nb, ntoread); 
        usleep(10000); 
        i-=128; 
        while(fgets(line, sizeof(line), fserial)); 
      }
      else 
      {
        if (nb!=fwrite(base64buf, 1,nb,fimage))
        {
          fprintf(stderr,"Write problem\n"); 
          fclose(fimage); 
          return 1; 
        }
  
        if (dump)
        {
          printf("%04x  ", offset+i ); 
          for (int j = 0; j < nb; j++) 
          {
            printf("%02x ", base64buf[j]); 
          }
          printf("\n") ; 
        }
      }
    }
  }

  fclose(fimage); 

  return 0;
}


