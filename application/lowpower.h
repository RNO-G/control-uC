#ifndef _RNO_G_LOWPOWER_H
#define _RNO_G_LOWPOWER_H

// 1 if low_power_mode is on, 0 otherwise 

extern volatile int low_power_mode; 

// Enters low power mode. This turns off the SBC and LTE, then the vicor. 
int low_power_mode_enter() ; 

#ifdef _RNO_G_REV_D
int low_power_mon_on(); 
int low_power_mon_off(); 
#endif

int low_power_mode_exit() ; 

int low_power_process(); 

int low_power_sleep_for_a_while(int howlong); 





#endif 
