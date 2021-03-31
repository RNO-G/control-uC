#include "application/time.h"
#include "hal_calendar.h" 
#include "shared/driver_init.h" 


uint32_t get_time() 
{
 return _calendar_get_counter(&CALENDAR.device) ; 
}


static uint32_t uptime_offset = 0; 
void set_time(uint32_t new_time)
{

  uint32_t current_time = get_time(); 
  uptime_offset += new_time-current_time; 
  _calendar_set_counter(&CALENDAR.device,new_time) ; 
}

void set_time_with_delta(int delta)
{
  set_time (get_time()+delta); 
}

uint32_t uptime() 
{
  return get_time() - uptime_offset; 
}
