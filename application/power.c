#include "power.h" 
#include "i2cbus.h" 
#include "hpl_calendar.h" 
#include "driver_init.h" 

#define LTC2992_ADDR 0x6f

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
#define TMP432_REG_STATUS 0x02  // ready only
#define TMP432_REG_ONESHOT 0x0f  // ready only
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


static uint32_t last_scheduled; 

static uint16_t get_adc(uint8_t reg_lsb, uint8_t reg_msb) 
{
  i2c_task_t msb = { .addr = LTC2992_ADDR, .write = 0, .reg = reg_msb } ; 
  i2c_task_t lsb = { .addr = LTC2992_ADDR, .write = 0, .reg = reg_lsb }; ; 
  i2c_enqueue(&msb); 
  i2c_enqueue(&lsb); 
  while (!lsb.done); 
  return read12bitADC(msb.data,lsb.data); 
}





int power_monitor_schedule() 
{
  last_scheduled = _calendar_get_counter(&CALENDAR.device); 
  // schedule a read of the power system 
  
  uint8_t ctrl_a_data = LTC2992_MASK_LTC_MODE_SNAPSHOT | LTC2992_MASK_LTC_SNAPSHOT_MEASURE_S12; 
  i2c_task_t ltc_task = {.addr = LTC2992_ADDR, .write=1, .reg =LTC2992_REG_CTRL_A, .data = ctrl_a_data }; 
  i2c_enqueue(&ltc_task); 

  //schedule a read of the temperatures 
  i2c_task_t tmp432_task = {.addr = TMP432_ADDRESS, .write=1, .reg = TMP432_REG_ONESHOT }; 
  i2c_enqueue(&tmp432_task); 
  return 0; 
}



int power_monitor_init() 
{

  //set temperature range, shutdown mode
  i2c_task_t tmp432_task = {.addr = TMP432_ADDRESS, .reg=TMP432_REG_STATUS, .write=1, .data = TMP432_EXTENDED_TEMPERATURE_MASK | TMP432_SD_MASK }; 
  i2c_enqueue(&tmp432_task); 

  //this will have a side effect of seetting up the LTC correctly 
  return power_monitor_schedule(); 


}

static uint16_t last_sense1; 
static uint16_t last_sense2; 
static uint16_t last_delta_sense1; 
static uint16_t last_delta_sense2; 
static uint32_t last_ltc_read; 

void update_ltc_if_ready() 
{

  //check status on power system 
  i2c_task_t check= {.addr=LTC2992_ADDR, .reg=LTC2992_ADC_STATUS, .write = 0 }; 
  i2c_enqueue(&check); 
  while (!check.done); 
  if ( (check.data & LTC2992_MASK_READY) == LTC2992_MASK_READY) //both ADC and IADC ready 
  {
    last_sense1 = get_adc(LTC2992_REG_S1_MSB, LTC2992_REG_S1_LSB);
    last_sense2 = get_adc(LTC2992_REG_S2_MSB, LTC2992_REG_S2_LSB);
    last_delta_sense1 = get_adc(LTC2992_REG_I1_MSB, LTC2992_REG_I1_LSB);
    last_delta_sense2 = get_adc(LTC2992_REG_I2_MSB, LTC2992_REG_I2_LSB); 
    last_ltc_read = last_scheduled; 
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

static uint32_t last_temp_read; 
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

int power_monitor_fill(power_system_monitor_t * state) 
{
  //check to make sure we're not busy 
  update_ltc_if_ready();
  update_temps_if_ready(); 

  //fill in the current/voltage monitors 
  state->PVv_cV = (last_sense1 *5) >> 1 ;  //25 mV / adc
  state->PVi_mA = (last_delta_sense1 *5) >> 1;   // 12.5 uV / adc, 5 mOhms
  state->BATv_cV = (last_sense2 *5) >> 1 ;  //25 mV / adc
  state->BATi_mA = (last_delta_sense2) *5;   // 12.5 uV / adc, 10 mOhms
  state->when_power = last_ltc_read; 

  state->local_T_C = last_local_t[1]; 
  state->remote1_T_C = last_remote1_t[1]; 
  state->remote2_T_C = last_remote2_t[1]; 

  state->local_T_sixteenth_C = last_local_t[0]; 
  state->remote1_T_sixteenth_C = last_remote1_t[0]; 
  state->remote2_T_sixteenth_C = last_remote2_t[0]; 

  state->when_temp = last_temp_read; 

  return 0; 
}


