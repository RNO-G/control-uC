#ifndef _rno_g_sbc_h
#define _rno_g_sbc_h

#include "include/rno-g-control.h" 



void sbc_init(); 
int sbc_process(int up); 

sbc_state_t sbc_get_state(); 
int sbc_turn_on(sbc_boot_mode_t boot_mode); 
int sbc_turn_off(); 





#endif
