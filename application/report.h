#ifndef _RNO_G_REPORT_H
#define _RNO_G_REPORT_H

#include "include/rno-g-control.h"

// may return a report if a new one was made 
const rno_g_report_t * report_process(int up, int * extrawake) ; 
void report_schedule(int navg); 
const rno_g_report_t * report_get(); 


#endif
