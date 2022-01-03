#ifndef _RNO_G_REPORT_H
#define _RNO_G_REPORT_H

#include "include/rno-g-control.h"

// may return a report if a new one was made 
#ifdef _RNO_G_REV_D
const rno_g_report_t * report_process(int up, uint32_t * extrawake) ; 
const rno_g_report_t * report_get(); 
#else
const rno_g_report_v2_t * report_process(int up, uint32_t * extrawake) ; 
const rno_g_report_v2_t * report_get(); 
#endif
void report_schedule(int navg); 


#endif
