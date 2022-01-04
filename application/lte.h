#ifndef _LTE_H
#define _LTE_H

#include "include/rno-g-control.h" 



lte_state_t lte_get_state(); 

int lte_init(); 
int lte_turn_on(int force); 
int lte_turn_off(); 
int lte_process(int up); 

const rno_g_lte_stats_t * lte_get_stats(); 

enum LTE_RESET_TYPE
{
  LTE_SOFT_CYCLE = 0, 
  LTE_HARD_CYCLE = 1, 
  LTE_POWER_CYCLE = 2, 
  LTE_FACTORY_RESET = 3, 
  LTE_NOT_A_RESET 
  
}; 

int lte_reset(int type); 


#endif
