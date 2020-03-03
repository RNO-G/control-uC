#ifndef _RNO_G_LORAWAN_H
#define _RNO_G_LORAWAN_H

/** LoRaWAN API  for RNO-G*/ 







/** initialize the lorawan system 
 *
 * This might take some arguments in the future
 * */ 
int lorawan_init(); 


/** Send a message */
int lorawan_send_message(uint8_t len, const uint8_t * msg) ; 


/** run the lorawan state machine for a bit */ 
int lorawan_process_events(); 

/** Get the next message in the queue, returning its size. */ 
uint8_t lorawan_next_received_message(uint8_t * msg);



#endif
