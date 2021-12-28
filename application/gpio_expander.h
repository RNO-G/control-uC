#include <stdint.h> 

typedef struct i2c_gpio_expander
{
  //ARGH, I wish C supported bitfield arrays 
  //so, these are just bitfields
  uint8_t surface_amps : 6; 
  uint8_t dh_amps : 3; 
  uint8_t radiant : 1;  
  uint8_t lt : 1;  
  uint8_t sbc : 1; 
} i2c_gpio_expander_t; 


/* Set the GPIO Expander State. 
 *  For each output, value of 1 makes it an output, and turns it on, while a value of 0 makes it an input (effectively turning it off) 
 *  The mask determines which outputs are affected by this.  
 */
int set_gpio_expander_state(i2c_gpio_expander_t value, i2c_gpio_expander_t  mask);

/* Retrieve the gpio expander state. If cached is 1, this just reads what we think it should be (doesn't actually query the gpio expander) */ 
int get_gpio_expander_state(i2c_gpio_expander_t * value, int cached);

/** Read the faults */ 
int get_gpio_expander_fault_state(i2c_gpio_expander_t * faults);

/** Initialize gpio expander state */ 
int gpio_expander_init(); 




