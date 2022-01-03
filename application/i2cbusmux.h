#ifndef _rno_g_i2c_busmux_h
#define _rno_g_i2c_busmux_h

#ifndef _RNO_G_REV_D

int i2c_busmux_init(void); 

enum i2c_busmux_bus
{
  I2C_BUSMUX_WINTER=1,
  I2C_BUSMUX_SUMMER=2,
  I2C_BUSMUX_BOTH=3 
};

#define i2c_busmux_quick_select(which) i2c_busmux_select(which,0) 
int i2c_busmux_select(int which, int force); 
int i2c_busmux_reset(void); 

#endif 
#endif 
