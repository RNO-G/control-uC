#include "lorawan.h" 
#include "config/lorawan_config.h" 


int lorawan_init() 
{

  //Do the equivalent of BoardInit


  return 0; 
}




// RX/TX buffer. This has both a maximum size and number of messages (due to bookkeeping).
// if each message were the same length, this would be simpler (could just do a circular buffer) 
//  to add, copy data to back of tx_buffer, and put the offset in tx_offsets, then adjust n_tx_messages and tx_buffer used
//  since we want a fifo, we have to read from the beginning. since we don't have a circular buffer we then need to copy 
//  rest of the messages back to the beginning, which sucks. but at least it's simple... 
static uint8_t tx_buffer[LORAWAN_TX_BUFFER_SIZE]; 
static uint16_t tx_offsets[LORAWAN_MAX_TX_MESSAGES]; 
static uint8_t rx_buffer[LORAWAN_TX_BUFFER_SIZE]; 
static uint16_t rx_offsets[LORAWAN_MAX_TX_MESSAGES]; 


struct msg_buffer 
{
  uint8_t * buffer; 
  uint16_t * offsets;
  uint16_t used; 
  uint8_t n_messages; 
}; 

static struct msg_buffer tx = {.buffer = tx_buffer, .offsets = tx_offsets, .used = 0, .n_messages = 0};
static struct msg_buffer rx = {.buffer = rx_buffer, .offsets = rx_offsets, .used = 0, .n_messages = 0};

static inline  uint8_t * first_message(struct msg_buffer * b) {return b->buffer; }

static int first_message_length(struct msg_buffer * b)
{
  return b->n_messages == 0 ? -1 : 
         b->n_messages == 1 ? b->used : 
         b->offsets[1]; 
}

static void consume_first_message(struct msg_buffer *b)
{
  //the simplest case
  if (b->n_messages == 0) return; 

  //the simple case
  if (b->n_messages == 1) 
  {
    b->n_messages=0 ;
    b->used = 0; 
    return; 
  }
  

  int len = first_message(b); 
  b->used-=len; 
  memmove(b->buffer, b->buffer+len, b->used); 
  b->n_messages--; 
  memmove(b->offsets, b->offsets+1, b->n_messages*sizeof(*b->offsets)); 
}

/** DOES NOT CHECK CAPACITY!!!! */ 
static void append_message(struct msg_buffer * b, int len, const uint8_t * msg) 
{
  b->offsets[b->n_messages] = b->used; 
  memcpy(b->offsets+b->used,msg, len); 
  b->n_messages++; 
  b->used+= len; 
}


static int have_tx_capacity(int len)
{
  return len+tx.used < LORAWAN_TX_BUFFER_SIZE && tx.n_messages < MAX_MESSAGES;
}
static int have_rx_capacity(int len)
{
  return len+rx.used < LORAWAN_RX_BUFFER_SIZE && rx.n_messages < MAX_MESSAGES;
}





int lorawan_send_message(uint8_t len, const uint8_t * msg) 
{

  if (!have_tx_capacity(len)) 
  {
    return -1; //tx buffer full
  }

  append_message(&tx, len, msg); 
  return 0; 
}


