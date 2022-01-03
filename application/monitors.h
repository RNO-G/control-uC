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
  MON_DOWN_3V3  
#ifdef _RNO_G_REV_D
  , MON_5V1,  // now in power in revE 
  MON_5V2 
#endif
} monitor_t ; 



/** For values read through the I2C switch, this must be called first. You may also need a delay due to capacitance */ 
void monitor_select(monitor_t what); 

/** reads the given voltage. This is synchronous (because getting the switches right otherwise be a huge pain) 
 **/ 

int16_t monitor(monitor_t what, int navg); 

/** Fill all analog monitor values */ 
#ifdef _RNO_G_REV_D
int monitor_fill(rno_g_monitor_t * m, int navg); 
#else
int monitor_fill(rno_g_report_v2_t * r, int navg); 
#endif

int monitor_init(); 
void monitor_deinit(); 




#endif
