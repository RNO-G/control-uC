#include "cmd_buf.h" 


void cmd_buf_init(cmd_buf_t * cmd_buf, int N, uint8_t * buf, struct io_descriptor * io)
{
  cmd_buf->N = N; 
  cmd_buf->buf = buf; 
  cmd_buf->token = '\n'; 
  cmd_buf->offset = 0;
  cmd_buf->io = io; 
  memset(cmd_buf->buf,0,cmd_buf->N); 
}

int cmd_buf_process(cmd_buf_t * cmd_buf) 
{
  io_read(cmd_buf->io, cmd_buf->buf+offset, cmd_buf->N-1-offset); 

  char * pos = strchr(cmd_buf->buf, cmd_buf->token); 
  while (lr) 
  {
    poslr = 0; 
    if (cmd_buf->cb(cmd_buf->buf, cmd_buf->data)) 
    {
      return 1;
    }
    else //eat the command by moving the rest of the buffer back
    {
      int len = pos - buf+1; 
      memmove(pos+1, cmd_buf->buf, len); 
      cmd_buf->offset -= len; 
      pos = strchr(cmd_buf->buf, cmd_buf->token); 
    }
  }

  if (cmd_buf->offset > cmd_buf->N/2)
  {
    memmove(cmd_buf->buf+cmd_buf->N/2, cmd_buf->buf, cmd_buf->N/2); 
    offset-=bufsize/2; 
    memset(cmd_buf->buf+cmd_buf/2, 0,cmd_buf/2); 
  }

  return 0; 
}
