#ifndef _RNO_G_REPORT_H
#define _RNO_G_REPORT_H

#include "include/rno-g-control.h"

// may return a report if a new one was made 
#ifdef _RNO_G_REV_D
#define RNO_G_REPORT_T rno_g_report_t
#else
#define RNO_G_REPORT_T rno_g_report_v2_t
#endif

const RNO_G_REPORT_T * report_process(int up, uint32_t * extrawake) ; 
const RNO_G_REPORT_T * report_get(); 
void report_schedule(int navg); 


#endif
