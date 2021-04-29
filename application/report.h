#ifndef _RNO_G_REPORT_H
#define _RNO_G_REPORT_H

#include "include/rno-g-control.h"

void report_process() ; 
void report_schedule(int navg); 
const rno_g_report_t * report_get(); 


#endif
