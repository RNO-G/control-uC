#ifndef _LTE_H
#define _LTE_H

#include "include/rno-g.h" 



lte_state_t lte_get_state(); 

int lte_init(); 
int lte_turn_on(); 
int lte_turn_off(); 
int lte_process(); 




#endif
