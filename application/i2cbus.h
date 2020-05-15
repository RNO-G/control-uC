#ifndef _rno_g_i2c_bus_h
#define _rno_g_i2c_bus_h

#include <stdint.h> 

void i2c_bus_init(); 


#define I2C_TASK_BUFFER_SIZE 16  

// low-level i2c expander functions 
// configures pins. bit = 1 means input (think i), bit = 0 means output (think o). 
//

#define I2C_EXPANDER_CONFIGURE_REGISTER 0x3
#define I2C_EXPANDER_INVERT_POLARITY_REGISTER  0x2
#define I2C_EXPANDER_SET_REGISTER  0x1
#define I2C_EXPANDER_GET_REGISTER  0x0

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


//Expander definitions. 
//verbose, but too lazy to come up with a better way

enum i2c_dir
{
  I2C_DIR_OUT, 
  I2C_DIR_IN
}; 



#define I2C_EXPANDER_A 0x38
#define I2C_EXPANDER_A_DIR I2C_DIR_OUT

#define I2C_EXPANDER_SURF_AMP_ENABLE_3_ADDR I2C_EXPANDER_A
#define I2C_EXPANDER_SURF_AMP_ENABLE_3_BIT 0
#define I2C_EXPANDER_SURF_AMP_ENABLE_4_ADDR I2C_EXPANDER_A
#define I2C_EXPANDER_SURF_AMP_ENABLE_4_BIT 1
#define I2C_EXPANDER_SURF_AMP_ENABLE_1_ADDR I2C_EXPANDER_A
#define I2C_EXPANDER_SURF_AMP_ENABLE_1_BIT 2
#define I2C_EXPANDER_SURF_AMP_ENABLE_5_ADDR I2C_EXPANDER_A
#define I2C_EXPANDER_SURF_AMP_ENABLE_5_BIT 3
#define I2C_EXPANDER_SURF_AMP_ENABLE_2_ADDR I2C_EXPANDER_A
#define I2C_EXPANDER_SURF_AMP_ENABLE_2_BIT 4
#define I2C_EXPANDER_SURF_AMP_ENABLE_6_ADDR I2C_EXPANDER_A
#define I2C_EXPANDER_SURF_AMP_ENABLE_6_BIT 5


#define I2C_EXPANDER_B 0x3A
#define I2C_EXPANDER_B_DIR I2C_DIR_OUT

#define I2C_EXPANDER_5V_ENABLE_1_ADDR I2C_EXPANDER_B
#define I2C_EXPANDER_5V_ENABLE_1_BIT 0 
#define I2C_EXPANDER_5V_ENABLE_2_ADDR I2C_EXPANDER_B
#define I2C_EXPANDER_5V_ENABLE_2_BIT 1 
#define I2C_EXPANDER_SBC_ENABLE_ADDR I2C_EXPANDER_B
#define I2C_EXPANDER_SBC_ENABLE_BIT 4 
#define I2C_EXPANDER_DH_AMP_ENABLE_2_ADDR I2C_EXPANDER_B
#define I2C_EXPANDER_DH_AMP_ENABLE_2_BIT 5
#define I2C_EXPANDER_DH_AMP_ENABLE_1_ADDR I2C_EXPANDER_B
#define I2C_EXPANDER_DH_AMP_ENABLE_1_BIT 6
#define I2C_EXPANDER_DH_AMP_ENABLE_3_ADDR I2C_EXPANDER_B
#define I2C_EXPANDER_DH_AMP_ENABLE_3_BIT 7


#define I2C_EXPANDER_C 0x3c 
#define I2C_EXPANDER_C_DIR I2C_DIR_IN

#define I2C_EXPANDER_SURF_AMP_FAULT_3_ADDR I2C_EXPANDER_C
#define I2C_EXPANDER_SURF_AMP_FAULT_3_BIT 0
#define I2C_EXPANDER_SURF_AMP_FAULT_4_ADDR I2C_EXPANDER_C
#define I2C_EXPANDER_SURF_AMP_FAULT_4_BIT 1
#define I2C_EXPANDER_SURF_AMP_FAULT_1_ADDR I2C_EXPANDER_C
#define I2C_EXPANDER_SURF_AMP_FAULT_1_BIT 2
#define I2C_EXPANDER_SURF_AMP_FAULT_5_ADDR I2C_EXPANDER_C
#define I2C_EXPANDER_SURF_AMP_FAULT_5_BIT 3
#define I2C_EXPANDER_SURF_AMP_FAULT_2_ADDR I2C_EXPANDER_C
#define I2C_EXPANDER_SURF_AMP_FAULT_2_BIT 4
#define I2C_EXPANDER_SURF_AMP_FAULT_6_ADDR I2C_EXPANDER_C
#define I2C_EXPANDER_SURF_AMP_FAULT_6_BIT 5


#define I2C_EXPANDER_D 0x3f
#define I2C_EXPANDER_D_DIR I2C_DIR_IN

#define I2C_EXPANDER_5V_FAULT_1_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_5V_FAULT_1_BIT 0 
#define I2C_EXPANDER_5V_FAULT_2_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_5V_FAULT_2_BIT 1 
#define I2C_EXPANDER_5V_FAULT_2_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_5V_FAULT_2_BIT 1 
#define I2C_EXPANDER_SBC_FAULT_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_SBC_FAULT_BIT 4 
#define I2C_EXPANDER_DH_AMP_FAULT_2_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_DH_AMP_FAULT_2_BIT 5
#define I2C_EXPANDER_DH_AMP_FAULT_1_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_DH_AMP_FAULT_1_BIT 6
#define I2C_EXPANDER_DH_AMP_FAULT_3_ADDR I2C_EXPANDER_D
#define I2C_EXPANDER_DH_AMP_FAULT_3_BIT 7



typedef struct i2c_task
{
  uint8_t addr : 7; // 7-bit address 
  uint8_t write : 1;  //write bit 
  uint8_t reg;  //register
  uint8_t data;  //either the data to write, or data will appear here when done
  volatile int8_t done ;  // set to 0 when task started, 1 when task is done,  negative if error. 
} i2c_task_t;


//busy wait for i2c queue to be done 
void i2c_queue_flush(); 
int i2c_queue_size(); 

/** Enqueue an i2c task. 
 *  If USE_SYNCHRONOUS_I2C is defined, then this is a misnomer and the task is completed instanly. 
 *
 *  Otherwise, 
 *  This must exist for the lifetime of the i2c operation! 
 *  So either you'll have to busy wait on done, or make it static, or whatever. 
 *
 * */ 
int i2c_enqueue( i2c_task_t * task); 





#endif
