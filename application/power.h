#ifndef _rno_g_power_h
#define _rno_g_power_h



struct power_system_state
{
  uint16_t PVv_cV; 
  uint16_t PVi_mA; 
  uint16_t BATv_cV; 
  uint16_t BATi_mA;  //
  int16_t T_dC; //deci-celsius
} power_system_state_t;

const power_system_state_t * power_system_state()

void schedule_power_system_read(); 






#endif
