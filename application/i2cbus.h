#ifndef _rno_g_i2c_bus_h
#define _rno_g_i2c_bus_h

#include <stdint.h> 

void i2c_bus_init(); 


#define I2C_TASK_BUFFER_SIZE 16  



enum i2c_task_flags
{
  I2CTSK_REG_LESS = 1 // this is for devices that just want a single byte, not more than one 
}; 


typedef struct i2c_task
{
  uint8_t addr : 7; // 7-bit address 
  uint8_t write : 1;  //write bit 
  uint8_t reg;  //register
  uint8_t data;  //either the data to write, or data will appear here when done
  volatile int8_t done ;  // set to 0 when task started, 1 when task is done,  negative if error. 
  uint8_t flags;  // 
} i2c_task_t;





//busy wait for i2c queue to be done 
void i2c_queue_flush(); 
int i2c_queue_size(); 


/** Enqueue an i2c task. 
 *  If USE_SYNCHRONOUS_I2C is defined, then this is a misnomer and the task is completed instanlty. 
 *
 *  Otherwise, 
 *  This must exist for the lifetime of the i2c operation! 
 *  So either you'll have to busy wait on done, or make it static, or whatever. 
 *
 * */ 
int i2c_enqueue( i2c_task_t * task); 


/** Probe i2c bus, starting at start_addr and ending at end_addr. bitset should be a a 16-element uint8_t array. 
 * Note that 0-0x07 and 0x78-0x7f are reserved so never probed.
 * if NULL is passed to bitset, found addresses will be printed to sbc_uart. 
 *
 * */ 
int i2c_detect(uint8_t start_addr, uint8_t end_addr, uint8_t * bitset); 


void i2c_bus_deinit(); 
int i2c_unstick(int ncycles); 

#endif
