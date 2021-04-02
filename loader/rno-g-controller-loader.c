#include <stdio.h> 
#include <stdlib.h> 
#include "shared/base64.h" 
#include <string.h>
#include <math.h>


int slot = 0; 
const char * device = "/dev/ttyO1" ;
const char * image = 0 ; 
FILE * fserial; 
int reset = 0;
int doitlive = 0; 


void usage() 
{
  printf("Usage: rno-g-controller-loader [-h] [-s SLOT=1] [-d DEVICE=/dev/ttyO1] [-r] [-l] image.bin\n"); 
  printf("   -h  Display this message\n");
  printf("   -s  SLOT  Pick the slot to write to (default 1). Note that slot 0 can only be used while in bootloader mode. \n");
  printf("   -r  Reset to slot after write. If slot is zero and not currently in bootloader, will reset into bootloader.  \n");
  printf("   -d  DEVICE The tty device (default /dev/ttyO1). Note that the device must already be configured appropriately, including converting \\n to \\r\\n on output. \n"); 
  printf("   -L  DO IT LIVE (don't verify)\n"); 
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
    else if (!strcmp(args[iarg],"-r")) reset =1; 
    else if (!strcmp(args[iarg],"-L")) doitlive =1; 
    else if (!strcmp(args[iarg],"-s"))
    {
      if (iarg++ == nargs) return 1; 
      slot = atoi(args[iarg]); 
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
}

char line[512]; 



const char* send_acked_command_with_attempts_and_b64bytes(const char * cmd, const char * args, int max_attempts_for_ack, int bytes_len, const uint8_t * bytes ) 
{
  fprintf(fserial,"%s %s",cmd,args); 

  if (bytes_len && bytes) 
  {
    base64_fprintf(fserial, bytes_len, bytes); 
  }
  else
  {
    fprintf(fserial,"\r\n"); 
  }

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
const char* send_acked_command_with_attempts(const char * cmd, const char * args, int max_attempts_for_ack) 
{
  send_acked_command_with_attempts_and_b64bytes(cmd,args,max_attempts_for_ack,0,NULL); 

}

const char * send_acked_command(const char * cmd, const char * args)
{
  return send_acked_command_with_attempts(cmd,args,3); 
}


int check_if_bootloader(int max_attempts) 
{

  const char * response = send_acked_command_with_attempts("#AM-I-BOOTLOADER","",max_attempts); 
  if (!response) return -1; 

  int ami; 
  sscanf(response,"#AM-I-BOOTLOADER %d\n",&ami); 
  return ami; 
}


int do_write(FILE * fimage, int start, int size) 
{
  fseek(fimage, start, SEEK_SET); 

  char cmdargs[64]; 
  uint8_t buf[48]; 
  for (unsigned i = 0; i < size; i+=sizeof(buf)) 
  {
    int rd = fread(buf,1,sizeof(buf), fimage); 
    snprintf(cmdargs, sizeof(cmdargs)," %d %d %d ", slot, i*sizeof(buf), rd); 
    if (!send_acked_command_with_attempts_and_b64bytes("#PROG-WRITE",cmdargs,3, rd, buf)); 
    {
      fprintf(stderr, "Failed to write!!! %s\n", cmdargs); 
      return 1; 
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

  fserial = fopen(device,"rw"); 
  if (!fserial) 
  {
    fprintf(stderr,"Could not open %s for r/w\n", device); 
    return 2; 
  }

  FILE * fimage = fopen(image,"r"); 
  if (!fimage) 
  {
    fprintf(stderr, "Could not open %s for reading\n", image); 
    fclose(fserial); 
    return 2; 
  }

  int in_bootloader = check_if_bootloader(3); 
  

  if (in_bootloader < 0) fprintf(stderr,"Could not determine if in bootloader\r\n"); 

  if (slot == 0 &&  !in_bootloader) 
  {
    if (reset) 
    {
      //TODO: hard reset into bootloader with gpios
      if(!send_acked_command("#RESET","10"))
      {
        fprintf(stderr,"Reset failed?\n"); 
        return 1; 
      }
    }
    else
    {
      fprintf(stderr,"Cannot write to slot 0 outside of bootloader!\n"); 
      return 1; 

    }
  }


  //figure out how big our image is so we know how much to erase 
  fseek(fimage, 0, SEEK_END); 
  size_t sz = ftell(fimage); 

  printf("Image is %lu bytes\n", sz); 
  //erase 

  int erase_size = 4096*ceil(sz/4096); 
  char cmdargs[128]; 
  snprintf(cmdargs,sizeof(cmdargs), "%d 0 %d", slot, erase_size); 
  if (!send_acked_command("#PROG-ERASE",cmdargs)) 
  {
      fprintf(stderr,"Erase failed?\n"); 
      return 1; 
  }

  //write 
  if (!do_write(fimage, 0, sz)) return 1; 

  //verify 

  if (!doitlive) 
  {
    fseek(fimage,0,SEEK_SET); 

    static char imgbuf[4096]; 

    int offset = 0; 
    while (!feof(fimage)) 
    {
      int rd = fread(imgbuf,1,4096, fimage); 
      int ok = 1; 

      for (int icheck = 0; icheck < rd; icheck+=128) 
      {
        int ncheck = icheck + 128 > rd ? rd-icheck : 128; 
        snprintf(cmdargs,sizeof(cmdargs)," %d %d %d", slot, offset + icheck*128, ncheck); 

        const char * response = send_acked_command("#PROG-READ", cmdargs); 
        if (!response) 
        {
          fprintf(stderr,"verify attempt failed %d\n", cmdargs); 
          return 1; 
        }

        int slot_check, offset_check, len_check; 
        char base64buf[256]; 
        sscanf(response,"#PROG-READ(slot=%d,offset=%d,len=%d): %s", &slot_check, &offset_check, &len_check, base64buf) ; 
        if(slot!=slot_check || offset!=offset_check || ncheck!=len_check)
        {
          fprintf(stderr, "verify arg mismatch %s vs. %s\n", cmdargs, response); 
          ok = 0; 
        }

        int nb = base64_decode(strlen(base64buf), base64buf); 
        if (nb != ncheck || memcmp(base64buf, imgbuf+icheck*128, ncheck))
        {
          ok =0;
        }
      }

      if (!ok) 
      {
        fprintf(stderr,"VERIFY MISMATCH at offset %d, trying to fix\n", offset); 
        do_write(fimage, offset, sz); 
        fseek(fimage,offset,SEEK_SET); 
      }
      else
      {
        offset+=rd; 
      }
    }
  }

  //reset if necessary 

  if (reset) 
  {
    snprintf(cmdargs,sizeof(cmdargs)," %d", slot); 
    if (!send_acked_command("#SYS-RESET",cmdargs))
    {
      fprintf(stderr, "Reset not acked\n"); 
      return 1; 
    }
  }



  return 0; 
}

