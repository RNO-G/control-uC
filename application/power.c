#include "power.h" 
#include "i2cbus.h" 

#define I2C_POWER_REG_CTRL_A 0x00  //control  register 1
#define I2C_POWER_REG_CTRL_B 0x01  // control regiser 2
#define I2C_POWER_REG_NADC 0x04  // set ADC resolution
#define I2C_POWER_REG_I1_MSB 0x14 //deltaSense1
#define I2C_POWER_REG_I1_LSB 0x15
#define I2C_POWER_REG_S1_MSB 0x1e //sense1 V 
#define I2C_POWER_REG_S1_LSB 0x1f
#define I2C_POWER_REG_I2_MSB 0x46 //deltaSense2
#define I2C_POWER_REG_I2_LSB 0x47
#define I2C_POWER_REG_S2_MSB 0x50 //sense2 V 
#define I2C_POWER_REG_S2_LSB 0x51
#define I2C_POWER_REG_GPIO_MSB 0x29 
#define I2C_POWER_REG_GPIO_LSB 0x29
#define I2C_POWER_MASK_LTC_ADC_RESOLUTION  0x80
#define I2C_POWER_MASK_LTC_MODE_SHUTDOWN  0x60
#define I2C_POWER_MASK_LTC_MODE_SINGLE_CYCLE  0x40
#define I2C_POWER_MASK_LTC_MODE_SNAPSHOT  0x20
#define I2C_POWER_MASK_LTC_MODE_CONTINUOUS  0x0



static power_system_state_t state; 
static i2c_task_t task
{ 
   .addr = , 
     




}
