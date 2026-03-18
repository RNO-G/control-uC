#ifndef _RNO_G_REPORT_H
#define _RNO_G_REPORT_H

#include "include/rno-g-control.h"


// may return a report if a new one was made 
const RNO_G_REPORT_T * report_process(int up, uint32_t * extrawake) ; 
const RNO_G_REPORT_T * report_get(); 
void report_schedule(int navg); 


#endif
