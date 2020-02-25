#ifndef _RNO_G_LORAWAN_H
#define _RNO_G_LORAWAN_H

/** LoRaWAN API  for RNO-G*/ 


#include "hal_spi_m_sync.h" 


typedef struct lorawan_context
{
  uint8_t * buffer;     //pointer to buffer for storing lora messages
  uint16_t buffer_size; //space to store received lora messages
  uint64_t euid; 
  uint64_t appid; 
  uint64_t appkey; 
  uint64_t channel_mask; 

  //note that the spi/ cs/ dio must already be initialized! 
  spi_m_sync_descriptor * spi; 
  uint32_t cs_pin; 
  uint32_t dio_pins[6]; 

} lorawan_context_t; 


/** initialize the lorawan system */ 
int lorawan_init(const lorawan_context_t * context); 


/** Send a message */
int lorawan_send_message(uint8_t len, const uint8_t * msg) ; 


/** run the lorawan state machine for a bit */ 
int lorawan_process_events(); 

/** Get the next message in the queue, returning its size. */ 
uint8_t lorawan_next_received_message(uint8_t * msg);



#endif
