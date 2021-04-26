#ifndef _rno_g_power_h
#define _rno_g_power_h

#include "rno-g-control.h" 


int power_monitor_init(); 

int power_monitor_fill(rno_g_power_system_monitor_t * state); 


/** schedules a read of the power system  */ 
int power_monitor_schedule(); 


#endif
