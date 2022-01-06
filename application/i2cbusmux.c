#include "application/i2cbusmux.h" 
#include "application/i2cbus.h" 
#include "shared/driver_init.h" 

//only available on REV E
#ifndef _RNO_G_REV_D
#define I2C_BUSMUX_ADDR 0x70 



int i2c_busmux_init(void) 
{
  
  return i2c_busmux_select(I2C_BUSMUX_BOTH,1); 

}

static i2c_task_t busmux_select = { .addr = I2C_BUSMUX_ADDR, .write = 1, .reg=0, .data = 0, .done = 1, .flags = I2CTSK_REG_LESS}; 
static int busmux_selected = 0; 

int i2c_busmux_reset(void) 
{
  gpio_set_pin_level(I2C_NRST, false); 
  gpio_set_pin_direction(I2C_NRST, GPIO_DIRECTION_OUT); 
  delay_us(1); 
  gpio_set_pin_direction(I2C_NRST, GPIO_DIRECTION_IN); 
  //we have to select agian 
  
  busmux_select.data = busmux_selected; 
  if (!i2c_enqueue(&busmux_select))
  {
    i2c_queue_flush(); 
    if (busmux_select.done == 1) 
    {
      return 0; 
    }
  }
  return -1; 
}



int i2c_busmux_select(int which, int force)
{
  if (!force && busmux_selected == which) return 0; 
  busmux_select.data = which;
  if (!i2c_enqueue(&busmux_select))
  {
    i2c_queue_flush(); 
    if (busmux_select.done==1) 
    {
      busmux_selected = which; 
      return 0; 
    }
  }
  return -1; 
}

#endif
