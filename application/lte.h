#ifndef _LTE_H
#define _LTE_H



typedef enum lte_state
{
  LTE_OFF, 
  LTE_TURNING_ON,
  LTE_ON,
  LTE_TURNING_OFF
} lte_state_t; 


lte_state_t lte_get_state(); 

int lte_init(); 
int lte_turn_on(); 
int lte_turn_off(); 
int lte_process(); 




#endif
