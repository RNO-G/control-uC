#include <stdio.h> 
#include <stdlib.h> 
#include "shared/base64.h" 
#include <string.h>
#include <math.h>
#include <errno.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/file.h>

int slot = 1; 
const char * device = "/dev/ttyO1" ;
const char * image = 0 ; 
FILE * fserial; 
int reset = 0;
int doitlive = 0; 
int verbose = 0; 


void usage() 
{
  printf("Usage: rno-g-controller-loader [-h] [-s SLOT=1] [-d DEVICE=/dev/ttyO1] [-r -L -v] image.bin\n"); 
  printf("   -h  Display this message\n");
  printf("   -s  SLOT  Pick the slot to write to (default 1). Note that slot 0 can only be used while in bootloader mode. \n");
  printf("   -r  Reset to slot after write. If slot is zero and not currently in bootloader, will reset into bootloader.  (if bootloader detected, will always reset)\n");
  printf("   -d  DEVICE The tty device (default /dev/ttyO1). Note that the device must already be configured appropriately, including converting \\n to \\r\\n on output. \n"); 
  printf("   -L  DO IT LIVE (don't verify)\n"); 
  printf("   -v  verbose mode)\n"); 
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
    else if (!strcmp(args[iarg],"-v")) verbose =1; 
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
  return 0; 
}

char line[512]; 



const char* send_acked_command_with_attempts_and_b64bytes(const char * cmd, const char * args, int max_attempts_for_ack, int bytes_len, const uint8_t * bytes ) 
{

  int nattempts = 0; 

  //clear the line
  while (fgets(line, sizeof(line), fserial)); 
  usleep(1000); 

  int retry_every = (max_attempts_for_ack / 4); 
  if (retry_every < 5) retry_every =5 ; 

  while (nattempts< max_attempts_for_ack) 
  {
    if (nattempts % retry_every == 0) 
    {
      fprintf(fserial,"%s%s%s",cmd,args && strlen(args) ? " " : "", args); 

      if (bytes_len && bytes) 
      {
        base64_fprintf(fserial, bytes_len, bytes); 
      }
      fprintf(fserial,"\n"); 
      usleep(5000); 
    }
    if (!fgets(line, sizeof(line), fserial))
    {
      usleep(nattempts*nattempts*2000); 
    }
    else if (prefix_matches(line,cmd))
    {
      if (verbose) printf("%s\n",line); 
      return line; 
    }
    else if (prefix_matches(line,"#ERR"))
    {
      fprintf(stderr,"%s -> %s %s\n",line, cmd,args); 
      usleep(15000);
    }
    nattempts++; 
  }
  return 0; 
}

const char* send_acked_command_with_attempts(const char * cmd, const char * args, int max_attempts_for_ack) 
{
  return send_acked_command_with_attempts_and_b64bytes(cmd,args,max_attempts_for_ack,0,NULL); 
}

const char * send_acked_command(const char * cmd, const char * args)
{
  return send_acked_command_with_attempts(cmd,args,20); 
}


int check_if_bootloader(int max_attempts) 
{

  const char * response = send_acked_command_with_attempts("#AM-I-BOOTLOADER","",max_attempts); 
  if (!response) return -1; 

  int ami= 0; 
  sscanf(response,"#AM-I-BOOTLOADER: %d\n",&ami); 
  return ami; 
}


int do_write(FILE * fimage, int start, int size) 
{
  fseek(fimage, start, SEEK_SET); 

  char cmdargs[48]; 
  uint8_t buf[48]; 
  for (unsigned i = 0; i < size; i+=sizeof(buf)) 
  {
    int rd = fread(buf,1,sizeof(buf), fimage); 
    snprintf(cmdargs, sizeof(cmdargs),"%d %d %d ", slot, start+i, rd); 
    if (!send_acked_command_with_attempts_and_b64bytes("#PROG-WRITE",cmdargs,20, rd, buf)) 
    {
      fprintf(stderr, "Failed to write!!! %s\n", cmdargs); 
      return 1; 
    }
    else
    {
      printf("."); 
      fflush(stdout); 

    }
  }
  printf("\n"); 
   

  return 0; 
}


int do_erase(int start, int size) 
{
  char cmdargs[128]; 
  snprintf(cmdargs,sizeof(cmdargs), "%d %d %d", slot, start,size); 
  if (!send_acked_command_with_attempts("#PROG-ERASE",cmdargs,100)) 
  {
      fprintf(stderr,"Erase failed?\n"); 
      return 1; 
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

  //lock the descriptor 
  if (flock(fd, LOCK_EX)) 
  {
    fprintf(stderr, "Could not get exclusive access\n"); 
    return 3; 
  }


  int flags = fcntl(fd, F_GETFL,0); 
  flags |=O_NONBLOCK; 
  if (fcntl(fd, F_SETFL,flags)) 
  {
    fprintf(stderr, "Could not set nonblock\n"); 
    return 4; 
  }

  FILE * fimage = fopen(image,"r"); 
  if (!fimage) 
  {
    fprintf(stderr, "Could not open %s for reading\n", image); 
    fclose(fserial); 
    return 2; 
  }

  int in_bootloader = check_if_bootloader(3); 
  

  if (in_bootloader < 0) 
  {
    fprintf(stderr,"Could not determine if in bootloader\n"); 
    exit(1); 
  }


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

  if (do_erase(0,erase_size)) return 1; 

  char cmdargs[128]; 
  snprintf(cmdargs,sizeof(cmdargs), "%d 0 %d", slot, erase_size); 
  if (!send_acked_command_with_attempts("#PROG-ERASE",cmdargs,100)) 
  {
      fprintf(stderr,"Erase failed?\n"); 
      return 1; 
  }

  //write 
  if (do_write(fimage, 0, sz)) return 1; 

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
        snprintf(cmdargs,sizeof(cmdargs),"%d %d %d", slot, offset + icheck, ncheck); 

        const char * response = send_acked_command("#PROG-READ", cmdargs); 
        if (!response) 
        {
          fprintf(stderr,"verify attempt failed %s\n", cmdargs); 
          return 1; 
        }

        int slot_check, offset_check, len_check; 
        uint8_t base64buf[256]; 
        sscanf(response,"#PROG-READ(slot=%d,offset=%d,len=%d): %s", &slot_check, &offset_check, &len_check, (char*) base64buf) ; 
        if(slot!=slot_check || offset+icheck!=offset_check || ncheck!=len_check)
        {
          fprintf(stderr, "verify arg mismatch %s vs. %s\n", cmdargs, response); 
          ok = 0; 
        }

        int nb = base64_decode(strlen((char*)base64buf), base64buf); 
        if (nb != ncheck || memcmp(base64buf, imgbuf+icheck, ncheck))
        {
          if (nb == ncheck) 
          {
            printf("img: "); 
            for (int i = 0; i < nb; i++) 
            {
              printf("%02x ", imgbuf[icheck+i]); 
            }
            printf("\ndev: "); 
            for (int i = 0; i < nb; i++) 
            {
              printf("%02x ", base64buf[i]); 
            }

            printf("\nmis: "); 

            for (int i = 0; i < nb; i++) 
            {
              printf("%s", base64buf[i]==imgbuf[icheck+i] ? "   " : "XX "); 
            }
            printf("\n"); 
 

          }
          else
          {
            printf("nb=%d, ncheck=%d\n", nb, ncheck); 
          }
          ok =0;
        }
      }

      if (!ok) 
      {
        fprintf(stderr,"VERIFY MISMATCH at offset %d, trying to fix\n", offset); 
        do_erase(offset, 4096); 
        usleep(1000); 
        do_write(fimage, offset, rd); 
        fseek(fimage,offset,SEEK_SET); 
      }
      else
      {
        offset+=rd; 
      }
    }
  }

  //reset if necessary 

  if (reset || in_bootloader) 
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

