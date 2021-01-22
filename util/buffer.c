#include "util/buffer.h" 


void buffer_init(buffer * buffer, int elem_size, int mem_size, void * mem)
{
  buffer->length = mem_size/elem_size;
  buffer->element_size = elem_size; 
  buffer->n_pushed = 0; 
  buffer->n_popped = 0; 
  buffer->mem = mem; 
}


int  buffer_occupancy(const buffer_t * buffer) 
{
  return buffer->npushed - buffer->n_popped; 
}

int buffer_capacity(const buffer_t * buffer) 
{
  return buffer->length - buffer_occupancy(buffer); 
}


int buffer_push(buffer_t * buffer, const void * element) 
{
  if (!buffer_capacity(buffer))
    return -1; 

BEGIN_CRITICAL_SECTION
  int position = buffer->n_pushed % buffer->length; 
  memcpy(buffer->mem + position*elem_size, element, buffer->elem_size); 
  buffer->n_pushed++; 
END_CRITICAL_SECTION

}

void * buffer_peak(buffer_t * buffeR) 
{
  int position = buffer->n_opped % buffer->length; 

}

