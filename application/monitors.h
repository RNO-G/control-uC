#ifndef __rno_g_uc_monitor_h
#define __rno_g_uc_monitor_h

#include "rno-g-control.h" 


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




/** reads the given voltage. This is synchronous (because getting the switches right otherwise be a huge pain) 
 * This means this might take a little while, but hopefuly that's ok. 
 *
 *   Units are mA for currents, cC (centiCelsius) for for temperatures. 
 **/ 

int16_t monitor(monitor_t what, int navg); 
int monitor_fill(rno_g_monitor_t * m, int navg); 

int monitor_init(); 




#endif
