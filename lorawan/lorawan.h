#ifndef _RNO_G_LORAWAN_H
#define _RNO_G_LORAWAN_H

/** LoRaWAN API  for RNO-G*/ 


#include <stdint.h> 
#include "rno-g-control.h" 


typedef enum e_lwan_msg_flags
{
  LORAWAN_MSG_CONFIRMED  = 1, 
} e_lwan_msg_flags_t; 





/** initialize the lorawan system 
 *
 * This might take some arguments in the future
 * */ 
int lorawan_init(int initial); 


typedef enum e_lwan_state
{
  LORAWAN_JOINING, 
  LORAWAN_READY, 
}lwan_state_t; 

lwan_state_t lorawan_state(void); 



int lorawan_tx_copy(uint8_t len, uint8_t opcode, const uint8_t* payload, uint8_t flags) ; 
int lorawan_tx_getmem(uint8_t len, uint8_t opcode, uint8_t ** payload, uint8_t flags) ; 
int lorawan_tx_push(); 



/** run the lorawan state machine for a bit 
 *  Returns  1 if we should inhibit sleeping 
 * */ 
int lorawan_process(int up); 

int lorawan_sched_timesync(); 

int lorawan_reset(); 

/** Copy the next message in the queue returning its size. */ 
uint8_t lorawan_copy_next_received_message(uint8_t * msg, uint8_t maxlen, uint8_t * port, uint8_t * flags);

/** Peek at the next message in the queue (no copy necessary), if there is one. 
 *
 * Returns the number of messages available (including this one), so if returns 0, no messages available. 
 *
 **/ 
int lorawan_rx_peek(uint8_t * len, uint8_t * port, uint8_t **msg, uint8_t *flags); 
/** Pop the message off the queue */ 
int lorawan_rx_pop(); 

void lorawan_stats(rno_g_lora_stats_t * stats); 


#endif
