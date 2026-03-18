#include "application/i2cbus.h" 
#include "application/monitors.h" 
#include "shared/driver_init.h" 
#include "application/lowpower.h" 
#include "hal_adc_sync.h" 
#include "hpl_calendar.h" 
#include "shared/printf.h" 
#include "application/time.h" 
#include <string.h> 
#include <stdint.h>



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



struct anamon
{
  int adc_channel;
  int i2c_addr;
  uint8_t monitor[8];
};
#ifdef _RNO_G_REV_N

#define AMON_LINES(AMON_LINE) \
  AMON_LINE(MON_SBC_5V, 1, 620, imon) \
  AMON_LINE(MON_SURF3V_1, 2, 1400, imon) \
  AMON_LINE(MON_SURF3V_2, 3, 1400, imon) \
  AMON_LINE(MON_DOWN_3V1, 4, 1400, imon) \
  AMON_LINE(MON_DOWN_3V2, 5, 1400, imon) \
  AMON_LINE(MON_DOWN_3V3, 6, 1400, imon) \
  AMON_LINE(MON_SURF3V_3, 7, 1400, imon)

#define BMON_LINES(BMON_LINE) \
  BMON_LINE(MON_RAIL_5V, 0, 2, vmon) \
  BMON_LINE(MON_RAIL_3V, 1, 2, vmon) \
  BMON_LINE(MON_LTE_3V, 3, 2, vmon)



#else


#define AMON_LINES(AMON_LINE) \
  AMON_LINE(MON_SURF3V_6, 2, 1400, imon)\
  AMON_LINE(MON_SURF3V_2, 3, 1400, imon)\
  AMON_LINE(MON_SURF3V_3, 4, 1400, imon)\
  AMON_LINE(MON_SURF3V_4, 5, 1400, imon)\
  AMON_LINE(MON_SURF3V_1, 6, 1400, imon)\
  AMON_LINE(MON_SURF3V_5, 7, 1400, imon)


#ifdef _RNO_G_REV_D
#define BMON_LINES(BMON_LINE) \
  BMON_LINE(MON_DOWN_3V1, 4, 1400, imon) \
  BMON_LINE(MON_DOWN_3V2, 6, 1400, imon) \
  BMON_LINE(MON_DOWN_3V3, 5, 1400, imon) \
  BMON_LINE(MON_SBC_5V, 7, 2, imon)
#else

#define BMON_LINES(BMON_LINE) \
  BMON_LINE(MON_RAIL_5V, 0,2, vmon) \
  BMON_LINE(MON_RAIL_3V, 1,2, vmon) \
  BMON_LINE(MON_LTE_3V, 3,2, vmon) \
  BMON_LINE(MON_DOWN_3V1, 4, 1400, imon) \
  BMON_LINE(MON_DOWN_3V2, 6, 1400, imon) \
  BMON_LINE(MON_DOWN_3V3, 5, 1400, imon) \
  BMON_LINE(MON_SBC_5V, 7, 2, imon)
#endif

#endif

#define ANAMON_LINE(MON,IDX, UNUSED, UNUSED2) [IDX] = MON, 
struct anamon mon_a = { 
  .adc_channel = ADC_MONA,
  .i2c_addr = 0x4c,
  .monitor = {
    AMON_LINES(ANAMON_LINE)
  }
};

struct anamon mon_b = { 
  .adc_channel = ADC_MONB,
  .i2c_addr = 0x4f, 
  .monitor = {
    BMON_LINES(ANAMON_LINE)
  }
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
  // do we need to wait a little bit here? 
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



  // this is referenced wrong, but changing the reference is annoying 
  // so instead we'll also measure the band gap (1.1 V)
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

  //ok, we're not using the right reference are we?
  //so let's just guestimate this adjustment
  float adj = 3.3/2;

  float T = 0;
  for (int i = 0; i < 2; i++)
  {
     T =  room_T + ( raw  *adj* est_1V/ 4095. - room_V) * (hot_T - room_T) / ( hot_V - room_V);
     if (i == 2) break; 
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

int16_t vmon(int input, int navg, float div)
{
  uint16_t raw = read_adc(input, navg);
  double v = raw * (3.3/4096)*div;
  return v *1000;
}



static void _mon_select(uint8_t a, uint8_t shift)
{
  i2c_task_t sel = {.addr = a, .write=1, .reg=0, .flags = I2CTSK_REG_LESS };
  sel.data = 1 << shift;
  i2c_enqueue(&sel);
  while (!sel.done);
}


#ifdef _RNO_G_REV_D

int monitor_fill(rno_g_monitor_t * m, int navg)
{
  int i;
  m->when = get_time() ; 


  for (i = 0; i < 6; i++) 
  {
    m->i_surf3v[i] = monitor(surf_map[i], navg); 
    monitor_select(surf_map[ (i+1) % 6] ); 
    delay_us(3000); 

    //ping pong between A and B
    switch(i) 
    {
      case 0: 
      case 1: 
        m->i_down3v[i] = monitor(dh_map[i], navg); 
        monitor_select(dh_map[i+1]); 
        break;
      case 2: 
        m->i_down3v[i] = monitor(dh_map[i], navg); 
        monitor_select(MON_SBC_5V); 
        break;
      case 3: 
        m->temp_cC = monitor(MON_TEMPERATURE,navg); 
        break;
      case 4: 
        m->i_sbc5v = monitor(MON_SBC_5V, navg); 
        monitor_select(MON_DOWN_3V1); 
        break; 
      case 5: 
       m->i_5v[0] = monitor(MON_5V1, navg);
       m->i_5v[1] = monitor(MON_5V2, navg);
    }
  }

  return 0; 
}

#else

int monitor_fill(RNO_G_REPORT_T * r, int navg)
{
  int when = get_time() ;
  r->analog_delta_when = when - r->when; 

  if (low_power_mode)
  {
    monitor_init();
    delay_ms(10); //adjust as needed...
  }

  static int16_t vals[MON_NUM_MON];
  float T = 0;

  for (int i = 0; i < 8; i++)
  {
    int inext = (i+1) % 8;
    for (int j = 0; j < 2; j++)
    {
      struct anamon * mon = j == 0 ? &mon_a : &mon_b;
      if (mon->monitor[i])
      {
        vals[mon->monitor[i]] = monitor(mon->monitor[i], navg);
      }
      else
      {
        if (!T) T = monitor_temperature(navg);
        else delay_us(1000);
      }

      monitor_select(mon->monitor[inext]);
      delay_us(3000);
    }
  }


  if (low_power_mode) monitor_deinit(); 

  //now asssign values
  
  r->i_sbc_div4 = vals[MON_SBC_5V] / 4;

  for (int i = 0; i < 3; i++)
  {
    r->i_dh_div4[i] = vals[MON_DOWN_3V1 + i] /4;
    r->i_surf_div4[i] = vals[MON_SURF3V_1 + i] /4;
  }

  r->V_5_div1p5 = vals[MON_RAIL_5V] / 1.5;
  r->V_33_div16= vals[MON_RAIL_3V]  / 16;
  r->V_lte_div16 = vals[MON_LTE_3V] / 16;


#ifndef _RNO_G_REV_N
  r->T_micro_times16 = T*16;
  for (int i = 3; i < 6; i++)
  {
    r->i_surf3v_div4[i] = vals[MON_SURF3V_1 + i] /4;
  }
#else
  r->T_micro_times2 = T*2;
#endif





  return 0;
}
#endif

void monitor_select(monitor_t what)
{
#define MON_SELECT_SWITCH_A(RAIL, CHANNEL, UNUSED, UNUSED2) case RAIL: _mon_select(mon_a.i2c_addr, CHANNEL); break;
#define MON_SELECT_SWITCH_B(RAIL, CHANNEL, UNUSED, UNUSED2) case RAIL: _mon_select(mon_b.i2c_addr, CHANNEL); break;
  switch (what) 
  {
    AMON_LINES(MON_SELECT_SWITCH_A)
    BMON_LINES(MON_SELECT_SWITCH_B)
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
#define SWITCH_AMON(RAIL, UNUSED, DIV, FN) case RAIL: return FN(mon_a.adc_channel, navg, DIV);
    AMON_LINES(SWITCH_AMON);
#define SWITCH_BMON(RAIL, UNUSED, DIV, FN) case RAIL: return FN(mon_b.adc_channel, navg, DIV);
    BMON_LINES(SWITCH_BMON);
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

















