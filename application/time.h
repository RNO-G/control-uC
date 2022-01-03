#ifndef _RNO_G_TIME_H
#define _RNO_G_TIME_H

#include <stdint.h> 

// if you just need the unix timestamp... 

int get_time(); 
int uptime(); //use if you need a monotonic time! 

void set_time(int new_time); 
void set_time_with_delta(int delta); 


#endif
