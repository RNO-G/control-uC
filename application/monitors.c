#include "application/i2cbus.h" 
#include "application/monitors.h" 
#include "shared/driver_init.h" 
#include "hal_adc_sync.h" 
#include "hpl_calendar.h" 
#include "shared/printf.h" 





typedef enum mon_a
{
  MON_A_SURF_3V_1 = 5, 
  MON_A_SURF_3V_2 = 3, 
  MON_A_SURF_3V_3 = 7, 
  MON_A_SURF_3V_4 = 6, 
  MON_A_SURF_3V_5 = 4, 
  MON_A_SURF_3V_6 = 2 
} mon_a_t; 


typedef enum mon_b
{
  MON_B_DWN_3V_1 = 4,  
  MON_B_DWN_3V_2 = 5,  
  MON_B_DWN_3V_3 = 6,  
  MON_B_SBC5 = 7
} mon_b_t; 


enum ADC_CHANNELS
{
  ADC_TEMP = 0, 
  ADC_MONA = 13,
  ADC_MONB = 12,
  ADC_MON_5V1 = 15,
  ADC_MON_5V2 = 14, 
  ADC_MON_ITEMP = 18
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
  return 0; 
}


int16_t monitor_temperature(int navg) 
{
  uint16_t raw = read_adc(ADC_TEMP, navg); 
  float v = raw * 3.3 / 4096; 


  float t = v > 2 ? 166.25-89*v : 
            v > 1.5 ? 161-86.4*v : 
            v > 1 ? 157.4-84*v : 
            154.4-80.76*v; 

  return (int16_t) (t*100); 
}


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

int monitor_fill(rno_g_monitor_t * m, int navg)
{
  int i;
  m->when = _calendar_get_counter(&CALENDAR.device); 

  for (i = 0; i < 6; i++) 
  {
    m->i_surf3v[i] = monitor(MON_SURF3V_1+i, navg); 
    monitor_select((MON_SURF3V_1+i+1) % 6 ); 
    delay_us(1000); 

    //alternate between the two to avoid capacitance issues
    if (i < 3) 
    {
      m->i_down3v[i] = monitor(MON_DOWN_3V1+i, navg); 
      if (i < 2) monitor_select(MON_DOWN_3V1+i+1); 
      else monitor_select(MON_SBC_5V); 
    }
    else if (i==3) 
    {
     m->temp_cC = monitor(MON_TEMPERATURE,navg); 
    }
    else if (i==5) 
    {
     m->i_5v[0] = monitor(MON_5V1, navg);
     m->i_5v[1] = monitor(MON_5V2, navg);
    }
    else if (i == 4) 
    {
      m->i_sbc5v = monitor(MON_SBC_5V, navg); 
      monitor_select(MON_DOWN_3V1); 
    }
    delay_us(2000); 
  }
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
      return monitor_temperature(navg); 
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
    case  MON_5V1: 
      return imon(ADC_MON_5V1,navg,620); 
    case  MON_5V2: 
      return imon(ADC_MON_5V2,navg,620); 
    default: 
      return -32768; 
  }
}

















