#ifndef _RNO_G_MODE_H
#define _RNO_G_MODE_H
#include "include/rno-g-control.h" 



//This figures out what mode we're in  (probably only useful after power on) 
void mode_init(); 

rno_g_mode_t mode_query(); 
int mode_set(const rno_g_mode_t); 




#endif
