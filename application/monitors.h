#ifndef __rno_g_uc_monitor_h
#define __rno_g_uc_monitor_h

#include "rno-g-control.h" 

/** These are the analog monitors! 
 *
 **/ 

typedef enum e_monitor
{
  MON_TEMPERATURE ,
  MON_SURF3V_1 , 
  MON_SURF3V_2 , 
  MON_SURF3V_3 , 
  MON_SURF3V_4 , 
  MON_SURF3V_5 , 
  MON_SURF3V_6 , 
  MON_SBC_5V  ,
  MON_DOWN_3V1 , 
  MON_DOWN_3V2 , 
  MON_DOWN_3V3 , 
  MON_5V1, 
  MON_5V2 
} monitor_t ; 



/** For values read through the I2C switch, this must be called first. You may also need a delay due to capacitance */ 
void monitor_select(monitor_t what); 

/** reads the given voltage. This is synchronous (because getting the switches right otherwise be a huge pain) 
 **/ 

int16_t monitor(monitor_t what, int navg); 

/** Fill all monitor values */ 
int monitor_fill(rno_g_monitor_t * m, int navg); 

int monitor_init(); 
void monitor_deinit(); 




#endif
