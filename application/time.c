#include "application/time.h"
#include "hal_calendar.h" 
#include "shared/driver_init.h" 


int get_time() 
{
 return _calendar_get_counter(&CALENDAR.device) ; 
}


static int uptime_offset = 0; 
void set_time(int new_time)
{

  uint32_t current_time = get_time(); 
  uptime_offset += new_time-current_time; 
  _calendar_set_counter(&CALENDAR.device,new_time) ; 
}

void set_time_with_delta(int delta)
{
  set_time (get_time()+delta); 
}

int uptime() 
{
  return get_time() - uptime_offset; 
}
