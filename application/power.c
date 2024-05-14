#include "power.h" 
#include "i2cbus.h" 
#include "driver_init.h" 
#include "config/config.h" 
#include "application/time.h" 

#define LTC2992_ADDR_TURBLTE 0x6a
#define LTC2992_ADDR_POWER 0x6f
#define LTC2992_ADDR_DIGI 0x6b

#define LTC2992_REG_CTRL_A 0x00  //control  register 1
#define LTC2992_REG_CTRL_B 0x01  // control regiser 2
#define LTC2992_REG_NADC 0x04  // set ADC resolution
#define LTC2992_REG_I1_MSB 0x14 //deltaSense1
#define LTC2992_REG_I1_LSB 0x15
#define LTC2992_REG_S1_MSB 0x1e //sense1 V 
#define LTC2992_REG_S1_LSB 0x1f
#define LTC2992_REG_I2_MSB 0x46 //deltaSense2
#define LTC2992_REG_I2_LSB 0x47
#define LTC2992_REG_S2_MSB 0x50 //sense2 V 
#define LTC2992_REG_S2_LSB 0x51
#define LTC2992_REG_GPIO_MSB 0x29 
#define LTC2992_REG_GPIO_LSB 0x29
#define LTC2992_MASK_LTC_ADC_RESOLUTION  0x80
#define LTC2992_MASK_LTC_SNAPSHOT_MEASURE_S12  0x06
#define LTC2992_MASK_LTC_MODE_SHUTDOWN  0x60
#define LTC2992_MASK_LTC_MODE_SINGLE_CYCLE  0x40
#define LTC2992_MASK_LTC_MODE_SNAPSHOT  0x20
#define LTC2992_MASK_LTC_MODE_CONTINUOUS  0x0
#define LTC2992_ADC_STATUS 0x32  //status register
#define LTC2992_MASK_READY  0xc0  // I think... the 2 MSB if i did this right


#define TMP432_ADDRESS 0x4d 
#define TMP432_REG_CFG1 0x09  // write-only  
#define TMP432_REG_STATUS 0x02  // read only
#define TMP432_REG_ONESHOT 0x0f  // write only
#define TMP432_REG_LOCAL_TEMP_HIGH 0x0
#define TMP432_REG_REMOTE1_TEMP_HIGH 0x1
#define TMP432_REG_REMOTE2_TEMP_HIGH 0x23
#define TMP432_REG_LOCAL_TEMP_LOW 0x29
#define TMP432_REG_REMOTE1_TEMP_LOW 0x10
#define TMP432_REG_REMOTE2_TEMP_LOW 0x24
#define TMP432_EXTENDED_TEMPERATURE_MASK 0x04  
#define TMP432_SD_MASK 0x40  
#define TMP432_BUSY_MASK 0x80  

static inline uint16_t read12bitADC(uint16_t msb, uint16_t lsb)
{
  return (( msb << 8) | lsb) >> 4; 

}

struct ltc2992_ctx 
{
  uint8_t addr; 
  int last_read; 
  uint16_t sense1;
  uint16_t sense2;
  uint16_t delta_sense1;
  uint16_t delta_sense2;
};

static struct ltc2992_ctx power_ctx = { .addr = LTC2992_ADDR_POWER };

#ifdef REV_AT_LEAST_E
static struct ltc2992_ctx digi_ctx = {.addr = LTC2992_ADDR_DIGI };
#endif

#ifdef REV_AT_LEAST_F
static struct ltc2992_ctx turblte_ctx = {.addr = LTC2992_ADDR_TURBLTE };
#endif





static int last_scheduled; 

static uint16_t get_adc(uint8_t address, uint8_t reg_msb, uint8_t reg_lsb) 
{
  i2c_task_t msb = { .addr = address, .write = 0, .reg = reg_msb } ; 
  i2c_task_t lsb = { .addr = address, .write = 0, .reg = reg_lsb }; ; 
  i2c_enqueue(&msb); 
  i2c_enqueue(&lsb); 
  while (!lsb.done); 
  return read12bitADC(msb.data,lsb.data); 
}





int power_monitor_schedule() 
{
  last_scheduled = get_time(); 

  // schedule a read of the power system 
  uint8_t ctrl_a_data = LTC2992_MASK_LTC_MODE_SNAPSHOT | LTC2992_MASK_LTC_SNAPSHOT_MEASURE_S12; 
  i2c_task_t ltc_task = {.addr = LTC2992_ADDR_POWER, .write=1, .reg =LTC2992_REG_CTRL_A, .data = ctrl_a_data }; 
  i2c_enqueue(&ltc_task); 

#ifdef REV_AT_LEAST_E
  //and the digitizers
  i2c_task_t ltc_digi_task = {.addr = LTC2992_ADDR_DIGI, .write=1, .reg =LTC2992_REG_CTRL_A, .data = ctrl_a_data }; 
  i2c_enqueue(&ltc_digi_task); 
#endif

#ifdef REV_AT_LEAST_F
  i2c_task_t ltc_turblte_task = { .addr = LTC2992_ADDR_TURBLTE, .write = 1, .reg = LTC2992_REG_CTRL_A, .data = ctrl_a_data}; 
  i2c_enqueue(&ltc_turblte_task); 
#endif

  //schedule a read of the temperatures 
  i2c_task_t tmp432_task = {.addr = TMP432_ADDRESS, .write=1, .reg = TMP432_REG_ONESHOT }; 
  i2c_enqueue(&tmp432_task); 

  while(!tmp432_task.done); 
  return 0; 
}



int power_monitor_init() 
{

  //set temperature range, shutdown mode
  i2c_task_t tmp432_task = {.addr = TMP432_ADDRESS, .reg=TMP432_REG_CFG1, .write=1, .data = TMP432_EXTENDED_TEMPERATURE_MASK | TMP432_SD_MASK }; 
  i2c_enqueue(&tmp432_task); 

  //this will have a side effect of seetting up the LTC correctly 
  return power_monitor_schedule(); 


}

void update_ltc_if_ready(struct ltc2992_ctx * c) 
{

  //check status on power system 
  i2c_task_t check= {.addr= c->addr, .reg=LTC2992_ADC_STATUS, .write = 0, .data = 0}; 
  i2c_enqueue(&check); 
  while (!check.done); 
  if ( (check.data & LTC2992_MASK_READY) == LTC2992_MASK_READY) //both ADC and IADC ready 
  {
    c->sense1 = get_adc(c->addr, LTC2992_REG_S1_MSB, LTC2992_REG_S1_LSB);
    c->sense2 = get_adc(c->addr, LTC2992_REG_S2_MSB, LTC2992_REG_S2_LSB);
    c->delta_sense1 = get_adc(c->addr, LTC2992_REG_I1_MSB, LTC2992_REG_I1_LSB);
    c->delta_sense2 = get_adc(c->addr, LTC2992_REG_I2_MSB, LTC2992_REG_I2_LSB); 
    c->last_read = last_scheduled; 
  }


  //check temperature status 
}
void get_temp(uint8_t high_reg, uint8_t low_reg, uint8_t * dest) 
{
  i2c_task_t task_high = {.addr=TMP432_ADDRESS, .reg = high_reg} ; 
  i2c_enqueue(&task_high);
  i2c_task_t task_low = {.addr=TMP432_ADDRESS, .reg = low_reg} ; 
  i2c_enqueue(&task_low);
  while (!task_high.done); 
  dest[0] = task_low.data; 
  dest[1] = task_high.data; 
}

static int last_temp_read; 
static uint8_t last_local_t[2]; 
static uint8_t last_remote1_t[2];
static uint8_t last_remote2_t[2];
void update_temps_if_ready() 
{
  i2c_task_t check = {.addr=TMP432_ADDRESS, .reg=TMP432_REG_STATUS }; 
  i2c_enqueue(&check); 
  while (!check.done); 
  if (check.done < 0) return; 
  if ((check.data & TMP432_BUSY_MASK) == 0) 
  {
    get_temp(TMP432_REG_LOCAL_TEMP_HIGH, TMP432_REG_LOCAL_TEMP_LOW, last_local_t);
    get_temp(TMP432_REG_REMOTE1_TEMP_HIGH, TMP432_REG_REMOTE1_TEMP_LOW, last_remote1_t);
    get_temp(TMP432_REG_REMOTE2_TEMP_HIGH, TMP432_REG_REMOTE2_TEMP_LOW, last_remote2_t);
    last_temp_read = last_scheduled; 
  }
}

#ifdef _RNO_G_REV_D
int power_monitor_fill(rno_g_power_system_monitor_t * state) 
#endif
#ifdef _RNO_G_REV_E
int power_monitor_fill(rno_g_report_v2_t * r) 
#endif
#ifdef _RNO_G_REV_F
int power_monitor_fill(rno_g_report_v3_t * r) 
#endif
{
  //check to make sure we're not busy 
  
  update_ltc_if_ready(&power_ctx);
#ifdef REV_AT_LEAST_E
  update_ltc_if_ready(&digi_ctx);
#endif
#ifdef REV_AT_LEAST_F
  update_ltc_if_ready(&turblte_ctx);
#endif

  update_temps_if_ready(); 

  //fill in the current/voltage monitors 
#ifdef _RNO_G_REV_D
  state->PVv_cV = (power_ctx.sense1 *5) >> 1 ;  //25 mV / adc
  state->PVi_mA = (power_ctx.delta_sense1 *5) >> 1;   // 12.5 uV / adc, 5 mOhms
  state->BATv_cV = (power_ctx.sense2 *5) >> 1 ;  //25 mV / adc
  state->BATi_mA = ((power_ctx.delta_sense2) *5 ) >> 2;   // 12.5 uV / adc, 10 mOhms
  state->when_power = power_ctx.last_read; 


  state->local_T_C = ((int)last_local_t[1])-64; 
  state->remote1_T_C = ((int)last_remote1_t[1])-64; 
  state->remote2_T_C = ((int)last_remote2_t[1])-64; 

  state->local_T_sixteenth_C = last_local_t[0]>>4; 
  state->remote1_T_sixteenth_C = last_remote1_t[0]>>4; 
  state->remote2_T_sixteenth_C = last_remote2_t[0]>>4; 

  state->when_temp = last_temp_read; 
#else
  r->V_pv_div25 = power_ctx.sense1;
  r->i_pv_div4p167 = power_ctx.delta_sense1;
  r->V_batt_div25 = power_ctx.sense2;
  r->i_batt_div1p25 = power_ctx.delta_sense2;

  r->V_radiant_div25 = digi_ctx.sense2;
  r->i_radiant_div3p125 = digi_ctx.delta_sense2;
  r->V_lt_div25 = digi_ctx.sense1;
  r->i_lt_div3p125 = digi_ctx.delta_sense1;

  r->T_local_times16 = 16 * (((int)last_local_t[1])-64)  + (last_local_t[0] >> 4); 
  r->T_remote_1_times16 = 16 * (((int)last_remote1_t[1])-64)  + (last_remote1_t[0] >> 4); 
  r->T_remote_2_times16 = 16 * (((int)last_remote2_t[1])-64)  + (last_remote2_t[0] >> 4); 
  r->temp_delta_when = last_temp_read - r->when  ;
  r->power_delta_when = power_ctx.last_read - r->when;
  r->digi_delta_when = digi_ctx.last_read - r->when;

#ifdef REV_AT_LEAST_F
  r->V_turb_div25 = turblte_ctx.sense1; 
  r->i_turb_div4p167 = turblte_ctx.delta_sense1; 
  r->V_lte_div25 = turblte_ctx.sense2; 
  r->i_lte_div3p125 = turblte_ctx.delta_sense2; 

  //end rev at least F 
#endif 



#endif



  return 0; 
}


