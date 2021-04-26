#ifndef _rno_g_sbc_h
#define _rno_g_sbc_h

#include "include/rno-g-control.h" 

extern rno_g_monitor_t last_mon; 
extern rno_g_power_system_monitor_t last_power; 


void sbc_init(); 
int sbc_io_process(); 

sbc_state_t sbc_get_state(); 
int sbc_turn_on(sbc_boot_mode_t boot_mode); 
int sbc_turn_off(); 





#endif
