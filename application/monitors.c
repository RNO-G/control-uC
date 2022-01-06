#include "application/i2cbus.h" 
#include "application/monitors.h" 
#include "shared/driver_init.h" 
#include "application/lowpower.h" 
#include "hal_adc_sync.h" 
#include "hpl_calendar.h" 
#include "shared/printf.h" 
#include "application/time.h" 
#include <string.h> 





typedef enum mon_a
{
  MON_A_SURF_3V_1 = 6, 
  MON_A_SURF_3V_2 = 3, 
  MON_A_SURF_3V_3 = 4, 
  MON_A_SURF_3V_4 = 5, 
  MON_A_SURF_3V_5 = 7, 
  MON_A_SURF_3V_6 = 2 
} mon_a_t; 

uint8_t surf_map[6] = { MON_SURF3V_1, MON_SURF3V_2, MON_SURF3V_3, MON_SURF3V_4,  MON_SURF3V_5, MON_SURF3V_6 }; 


typedef enum mon_b
{
  MON_B_DWN_3V_1 = 4,  
  MON_B_DWN_3V_2 = 6,  
  MON_B_DWN_3V_3 = 5,  
  MON_B_SBC5 = 7
} mon_b_t; 

uint8_t dh_map[3] = { MON_DOWN_3V1, MON_DOWN_3V2, MON_DOWN_3V3 }; 

enum ADC_CHANNELS
{
#ifdef _RNO_G_REV_D
  ADC_TEMP = 0, 
#endif
  ADC_MONA = 13,
  ADC_MONB = 12,
#ifdef _RNO_G_REV_D
  ADC_MON_5V1 = 15,
  ADC_MON_5V2 = 14, 
#endif
  ADC_MON_ITEMP = 0x18
};


#define NSKIP 2 
static uint16_t read_adc(int chan, int navg) 
{
  uint32_t sum = 0; 
  uint8_t buf[2] = {0,0}; 
  int i = 0; 
  if (navg < 1) navg = 1; 
  adc_sync_set_inputs(&ANALOGIN, chan, 0x18,0); 
  adc_sync_enable_channel(&ANALOGIN, 0); 
  while (i < navg+NSKIP) 
  {
    adc_sync_read_channel(&ANALOGIN, 0, (uint8_t*) &buf, 2); 
    if (i > NSKIP-1) //skip first NSKIP
      sum+= buf[0] + (buf[1] << 8); 
    i++; 
  }
  adc_sync_disable_channel(&ANALOGIN, 0); 
  return sum/=navg; 
}



int monitor_init()
{
	adc_sync_init(&ANALOGIN, ADC, (void *)NULL);
  return 0; 
}


void monitor_deinit() 
{
  adc_sync_deinit(&ANALOGIN); 

}


/** on REV_D, this is the on-board temperature sensor
 * */ 
#ifdef _RNO_G_REV_D
static float monitor_temperature(int navg) 
{
  uint16_t raw = read_adc(ADC_TEMP, navg); 
  float v = raw * 3.3 / 4096; 


  float t = v > 2 ? 166.25-89*v : 
            v > 1.5 ? 161-86.4*v : 
            v > 1 ? 157.4-84*v : 
            154.4-80.76*v; 

  return t; 
}
#else
// otherwise, let's monitor the internal temperature? 
// See section 37.11.8.2 of datasheet
static float monitor_temperature(int navg) 
{

  uint16_t raw = read_adc(ADC_MON_ITEMP, navg); 

  static uint64_t calib = 0; 
  static float room_T ; 
  static float hot_T; 

  static float room_1V; 
  static float hot_1V; 
  static float room_V; 
  static float hot_V; 
  if (!calib) 
  {
    memcpy(&calib, (uint64_t*) 0x00806030, sizeof(calib)); 
    room_T = (calib & 0xff)  + 0.1 * ( (calib >> 8) & 0xf); 
    hot_T = ((calib >> 12)  & 0xff)  + 0.1 * ( (calib >> 20) & 0xf); 
    int8_t room_1V_diff = (calib >> 24 ) & 0xff; 
    int8_t hot_1V_diff = (calib >> 32 ) & 0xff; 
    room_1V = 1. - 0.001 * room_1V_diff;
    hot_1V = 1. - 0.001 * hot_1V_diff;
    uint16_t room_val = (calib >> 40) & 0xfff; 
    uint16_t hot_val = (calib >> 52) & 0xfff; 

    room_V = room_val * room_1V / 4095.;
    hot_V = hot_val * hot_1V / 4095.;
  }
  float est_1V =1 ; 

  float T = 0;
  for (int i = 0; i < 2; i++) 
  {
     T = ( raw  * est_1V/ 4095. - room_V) * (hot_T - room_T) / ( hot_V - room_V); 
     if (i == 1) break; 
     est_1V = room_1V + ( hot_1V - room_1V) * (T - room_T) / (hot_T - room_T); 
  }

  return T;
}


#endif


int16_t imon(int input, int navg, int R) 
{
  uint16_t raw = read_adc (input, navg); 
  double v = raw * (3.3/4096); //3.3 V effective reference
  return v / (276e-9*R) ;   //276 uA/A) 
}



static void _mon_select(uint8_t a, uint8_t shift) 
{
  i2c_task_t sel = {.addr = a, .write=1, .reg=0, .flags = I2CTSK_REG_LESS }; 
  sel.data = 1 << shift; 
  i2c_enqueue(&sel); 
  while (!sel.done); 
}

static void mon_a_select(monitor_t what) 
{
  int shift =
    what == MON_SURF3V_1 ? MON_A_SURF_3V_1 : 
    what == MON_SURF3V_2 ? MON_A_SURF_3V_2 : 
    what == MON_SURF3V_3 ? MON_A_SURF_3V_3 : 
    what == MON_SURF3V_4 ? MON_A_SURF_3V_4 : 
    what == MON_SURF3V_5 ? MON_A_SURF_3V_5 : 
                           MON_A_SURF_3V_6 ;  

  _mon_select(0x4c, shift); 
}


static void mon_b_select(monitor_t what) 
{
  int shift =
    what == MON_DOWN_3V1 ? MON_B_DWN_3V_1 : 
    what == MON_DOWN_3V2 ? MON_B_DWN_3V_2 : 
    what == MON_DOWN_3V3 ? MON_B_DWN_3V_3 : 
                           MON_B_SBC5;

  _mon_select(0x4f,shift); 
}

#ifdef _RNO_G_REV_D
int monitor_fill(rno_g_monitor_t * m, int navg)
#else
int monitor_fill(rno_g_report_v2_t * r, int navg)
#endif
{
  int i;
  int when = get_time() ; 


#ifdef _RNO_G_REV_D
  const int max = 6; 
  m->when = when; 
#else
  const int max = 5; 
  r->analog_delta_when = when - r->when; 
  if (low_power_mode) monitor_init(); 
#endif

  for (i = 0; i < max; i++) 
  {
#ifdef _RNO_G_REV_D
    m->i_surf3v[i] = monitor(surf_map[i], navg); 
#else
    r->i_surf_div4[i] = monitor(surf_map[i], navg) >> 2; 
#endif
    monitor_select(surf_map[ (i+1) % 6] ); 
    delay_us(3000); 

    //alternate between the two to avoid capacitance issues
    if (i < 3) 
    {
#ifdef _RNO_G_REV_D
      m->i_down3v[i] = monitor(dh_map[i], navg); 
#else
      r->i_dh_div4[i] = monitor(dh_map[i], navg) >> 2; 
#endif
      if (i < 2) monitor_select(dh_map[i+1]); 
      else monitor_select(MON_SBC_5V); 
    }
    else if (i==3) 
    {
#ifdef _RNO_G_REV_D
     m->temp_cC = monitor(MON_TEMPERATURE,navg); 
#else
     float T = monitor_temperature(navg); 
     r->T_local_times16 = 16*T; 
#endif
    }
#ifdef _RNO_G_REV_D
    else if (i==5) 
    {
     m->i_5v[0] = monitor(MON_5V1, navg);
     m->i_5v[1] = monitor(MON_5V2, navg);
    }
#endif
    else if (i == 4) 
    {
#ifdef _RNO_G_REV_D
      m->i_sbc5v = monitor(MON_SBC_5V, navg); 
#else
      r->i_sbc_div4 = monitor(MON_SBC_5V,navg) >> 2; 
#endif 
      monitor_select(MON_DOWN_3V1); 
    }
    if ( i < max-1) 
      delay_us(3000); 
  }

#ifndef _RNO_G_REV_D

  if (low_power_mode) monitor_deinit(); 
#endif
  return 0; 
}

void monitor_select(monitor_t what)
{
  switch (what) 
  {
    case MON_SURF3V_1:
    case MON_SURF3V_2:
    case MON_SURF3V_3:
    case MON_SURF3V_4:
    case MON_SURF3V_5:
    case MON_SURF3V_6:
      mon_a_select(what); 
      break;
    case MON_SBC_5V: 
    case MON_DOWN_3V1: 
    case MON_DOWN_3V2: 
    case MON_DOWN_3V3: 
      mon_b_select(what); 
    default: 
      break; 
  }
}

int16_t monitor(monitor_t what, int navg) 
{
  switch (what) 
  {

    case MON_TEMPERATURE:
      return 100*monitor_temperature(navg); 
    case MON_SURF3V_1:
    case MON_SURF3V_2:
    case MON_SURF3V_3:
    case MON_SURF3V_4:
    case MON_SURF3V_5:
    case MON_SURF3V_6:
      return imon(ADC_MONA, navg, 1400); 
    case MON_SBC_5V: 
      return imon(ADC_MONB,navg,620); 
    case MON_DOWN_3V1: 
    case MON_DOWN_3V2: 
    case MON_DOWN_3V3: 
      return imon(ADC_MONB,navg,1400); 
#ifdef _RNO_G_REV_D
    case  MON_5V1: 
      return imon(ADC_MON_5V1,navg,620); 
    case  MON_5V2: 
      return imon(ADC_MON_5V2,navg,620); 
#endif
    default: 
      return -32768; 
  }
}

















