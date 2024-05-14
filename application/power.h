#ifndef _rno_g_power_h
#define _rno_g_power_h

#include "rno-g-control.h" 


int power_monitor_init(); 

#ifdef _RNO_G_REV_D
int power_monitor_fill(rno_g_power_system_monitor_t * state); 
#endif
#ifdef _RNO_G_REV_E
int power_monitor_fill(rno_g_report_v2_t * report); 
#endif
#ifdef _RNO_G_REV_F
int power_monitor_fill(rno_g_report_v3_t * report); 
#endif



/** schedules a read of the power system  */ 
int power_monitor_schedule(); 


#endif
