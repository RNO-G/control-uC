#ifndef _LTE_H
#define _LTE_H

#include "include/rno-g-control.h" 



lte_state_t lte_get_state(); 

int lte_init(); 
int lte_turn_on(int force); 
int lte_turn_off(); 
int lte_process(int up); 

const rno_g_lte_stats_t * lte_get_stats(); 

int lte_reset(rno_g_lte_reset_type_t type); 


#endif
