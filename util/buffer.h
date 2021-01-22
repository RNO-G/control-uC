#ifndef _rno_g_buffer_h
#define _rno_g_buffer_h

struct buffer
{
  uint16_t length; 
  uint16_t element_size; 
  volatile uint32_t n_pushed; 
  volatile uint32_t n_popped; 
  void * mem; 
} buffer_t; 

void buffer_init(buffer_t * buffer, int elem_size, int mem_size, void * mem);
int buffer_push(buffer_t *buffer, const void * element); 
void * buffer_peek (buffer_t *buffer); 
void buffer_pop (buffer_t * buffer, void * element); 
int buffer_occupancy(const buffer_t * buffer); 
int buffer_capacity(const buffer_t * buffer); 



#endif
