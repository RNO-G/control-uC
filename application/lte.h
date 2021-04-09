#ifndef _LTE_H
#define _LTE_H

#include "include/rno-g-control.h" 



lte_state_t lte_get_state(); 

int lte_init(); 
int lte_turn_on(int force); 
int lte_turn_off(); 
int lte_process(); 




#endif
