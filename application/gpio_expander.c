#include "application/gpio_expander.h" 
#include "application/i2cbus.h" 
#include "config/config.h" 


#define I2C_EXPANDER_CONFIGURE_REGISTER 0x3
#define I2C_EXPANDER_INVERT_POLARITY_REGISTER  0x2
#define I2C_EXPANDER_SET_REGISTER  0x1
#define I2C_EXPANDER_GET_REGISTER  0x0

//note weird convention
enum i2c_expander_dir
{
  I2C_DIR_OUT, 
  I2C_DIR_IN
};

#ifdef I2C_EXPANDER_AUTOPROBE
static uint8_t autoprobe_addr_expander[4]; 
static uint8_t autoprobe_addr_expander_success; 
const uint8_t autoprobe_addr_expander_options[4][2] = 
  {{ 0x20, 0x38 }, 
   { 0x22, 0x3A }, 
   { 0x24, 0x3C }, 
   { 0x27, 0x3F }}; 


static void autoprobe_i2c_expander(void) 
{
  i2c_task_t probe = {.addr = 0, .write = 0, .reg=I2C_EXPANDER_SET_REGISTER, .data = 0, .done = 1}; 
  for (unsigned i = 0; i < 4; i++) 
  {
    for (unsigned j = 0; j < 2; j++) 
    {
      probe.addr = autoprobe_addr_expander_options[i][j]; 
      i2c_enqueue(&probe); 
      i2c_queue_flush(); 
      if (probe.done > 0) 
      {
        autoprobe_addr_expander_success |=  1 << i; 
        autoprobe_addr_expander[i] = probe.addr; 
      }
    }
  }
}

#define I2C_EXPANDER_A (autoprobe_addr_expander[0])
#define I2C_EXPANDER_B (autoprobe_addr_expander[1])
#define I2C_EXPANDER_C (autoprobe_addr_expander[2])
#define I2C_EXPANDER_D (autoprobe_addr_expander[3])

//not probing
#else
#ifdef _RNO_G_REV_D
#define I2C_EXPANDER_A 0x38
#define I2C_EXPANDER_B 0x3A
#define I2C_EXPANDER_C 0x3c 
#define I2C_EXPANDER_D 0x3f
#else
#define I2C_EXPANDER_A 0x20
#define I2C_EXPANDER_B 0x22
#define I2C_EXPANDER_C 0x24 
#define I2C_EXPANDER_D 0x27
#endif

//not probing
#endif

#define I2C_EXPANDER_SURF_AMP_3_BIT 0
#define I2C_EXPANDER_SURF_AMP_4_BIT 1
#define I2C_EXPANDER_SURF_AMP_1_BIT 2
#define I2C_EXPANDER_SURF_AMP_5_BIT 3
#define I2C_EXPANDER_SURF_AMP_2_BIT 4
#define I2C_EXPANDER_SURF_AMP_6_BIT 5


#define I2C_EXPANDER_5V_1_BIT 0 
#define I2C_EXPANDER_5V_2_BIT 1 
#define I2C_EXPANDER_SBC_BIT 4 
#define I2C_EXPANDER_DH_AMP_2_BIT 5
#define I2C_EXPANDER_DH_AMP_1_BIT 6
#define I2C_EXPANDER_DH_AMP_3_BIT 7

#define I2C_EXPANDER_J29_BIT 6
#define I2C_EXPANDER_EXT_BUS_BIT 7




//these keep the state 

static i2c_task_t A_state  = { .addr = 0, .write =1, .reg=I2C_EXPANDER_SET_REGISTER, .data = 0xff, .done = 1}; 
static i2c_task_t B_state  = { .addr = 0, .write =1, .reg=I2C_EXPANDER_SET_REGISTER, .data = 0xff, .done = 1}; 

//this is used to actually perform a query. We query the direction, which will be out if on, in if not in all cases (even if polarity is inverted) 
static i2c_task_t A_query_dir  = { .addr = 0, .write =0, .reg=I2C_EXPANDER_CONFIGURE_REGISTER, .data = 0x0, .done = 1}; 
static i2c_task_t B_query_dir  = { .addr = 0, .write =0, .reg=I2C_EXPANDER_CONFIGURE_REGISTER, .data = 0x0, .done = 1}; 

static i2c_task_t A_query_state  = { .addr = 0, .write =0, .reg=I2C_EXPANDER_SET_REGISTER, .data = 0x0, .done = 1}; 
static i2c_task_t B_query_state  = { .addr = 0, .write =0, .reg=I2C_EXPANDER_SET_REGISTER, .data = 0x0, .done = 1}; 



//used to set direction
static i2c_task_t A_dir = { .addr = 0, .write = 1, .reg=I2C_EXPANDER_CONFIGURE_REGISTER, .data = 0xff, .done = 1}; 
static i2c_task_t B_dir = { .addr = 0, .write = 1, .reg=I2C_EXPANDER_CONFIGURE_REGISTER, .data = 0xff, .done = 1}; 


//these are only used for reads
static i2c_task_t C =  { .addr = 0, .write = 0, .reg=I2C_EXPANDER_GET_REGISTER, .data = 0x0, .done = 1} ;
static i2c_task_t D = { .addr = 0, .write = 0, .reg=I2C_EXPANDER_GET_REGISTER, .data = 0x0, .done = 1} ; 


static const uint8_t surf_amp_map[6] = {
  I2C_EXPANDER_SURF_AMP_1_BIT, 
  I2C_EXPANDER_SURF_AMP_2_BIT, 
  I2C_EXPANDER_SURF_AMP_3_BIT, 
  I2C_EXPANDER_SURF_AMP_4_BIT, 
  I2C_EXPANDER_SURF_AMP_5_BIT, 
  I2C_EXPANDER_SURF_AMP_6_BIT, 
}; 


static const uint8_t dh_amp_map[3] = {
  I2C_EXPANDER_DH_AMP_1_BIT, 
  I2C_EXPANDER_DH_AMP_2_BIT, 
  I2C_EXPANDER_DH_AMP_3_BIT, 
}; 


int gpio_expander_init() 
{
#ifdef I2C_EXPANDER_AUTOPROBE
  autoprobe_i2c_expander(); 
#endif

  A_state.addr = I2C_EXPANDER_A;
  A_query_dir.addr = I2C_EXPANDER_A;
  A_dir.addr = I2C_EXPANDER_A;
  A_query_state.addr = I2C_EXPANDER_A;
  B_state.addr = I2C_EXPANDER_B;
  B_query_dir.addr = I2C_EXPANDER_B;
  B_dir.addr = I2C_EXPANDER_B;
  B_query_state.addr = I2C_EXPANDER_B;
  C.addr = I2C_EXPANDER_C;
  D.addr = I2C_EXPANDER_D;

  return get_gpio_expander_state(0,0); 
}


//yuck
int set_gpio_expander_state(i2c_gpio_expander_t value, i2c_gpio_expander_t mask) 
{
  //check to make sure we don't need to wait for previous command to take
#ifdef _RNO_G_REV_D
  int need_A =!! (mask.surface_amps ); 
#else
  int need_A =!! (mask.surface_amps || mask.j29 || mask.ext_bus ); 
#endif
  int need_B =!! (mask.dh_amps || mask.radiant || mask.lt || mask.sbc); 

  if ( (need_A && !A_state.done) || (need_B && ! B_state.done))
  {
    i2c_queue_flush(); 
  }

  // 

#define GPIO_EXPANDER_SET_OUTPUT(exp, bit, val)\
  if (val) { exp##_state.data |= (1 << bit); exp##_dir.data &= ~(1 << bit); }\
  else { exp##_dir.data |= (1 << bit); exp##_state.data &= ~(1 << bit); }

#define GPIO_EXPANDER_SET_INV_OUTPUT(exp, bit, val)\
  if (val) { exp##_state.data &= ~(1 << bit); exp##_dir.data &= ~(1 << bit); }\
  else { exp##_dir.data |= (1 << bit); exp##_state.data |= (1 << bit); }



  if (need_A) 
  {


    for (int i = 0; i < 6; i++) 
    {
      if (mask.surface_amps & (1 << i)) 
      {
        GPIO_EXPANDER_SET_OUTPUT(A,surf_amp_map[i], value.surface_amps & (1 << i)) 
      }
    }

#ifndef _RNO_G_REV_D
    if (mask.j29) 
    {
      GPIO_EXPANDER_SET_OUTPUT(A, I2C_EXPANDER_J29_BIT, value.j29)
    }
    if (mask.ext_bus)
    {
      GPIO_EXPANDER_SET_INV_OUTPUT(A, I2C_EXPANDER_EXT_BUS_BIT, value.ext_bus)
    }

#endif

    //first set the value, then the direction
    i2c_enqueue(&A_state);
    i2c_enqueue(&A_dir);
   
  }
 
  if (need_B) 
  {
    for (int i = 0; i < 3; i++) 
    {
      if (mask.dh_amps & (1 << i)) 
      {
        GPIO_EXPANDER_SET_OUTPUT(B,dh_amp_map[i], value.dh_amps & (1 << i)) 
      }
    }

    if (mask.sbc) 
    {
      GPIO_EXPANDER_SET_OUTPUT(B, I2C_EXPANDER_SBC_BIT, value.sbc) 
    }

    if (mask.radiant) 
    {
#ifdef _RNO_G_REV_D
      GPIO_EXPANDER_SET_OUTPUT(B, I2C_EXPANDER_5V_1_BIT, value.radiant) 
#else
      GPIO_EXPANDER_SET_INV_OUTPUT(B, I2C_EXPANDER_5V_1_BIT, value.radiant) 
#endif
    }

    if (mask.lt) 
    {
#ifdef _RNO_G_REV_D
      GPIO_EXPANDER_SET_OUTPUT(B, I2C_EXPANDER_5V_2_BIT, value.lt) 
#else
      GPIO_EXPANDER_SET_INV_OUTPUT(B, I2C_EXPANDER_5V_2_BIT, value.lt) 
#endif
    }

    //first set the value, then the direction
    i2c_enqueue(&B_state);
    i2c_enqueue(&B_dir);
  }

  return 0;
}


int get_gpio_expander_state(i2c_gpio_expander_t * value,  int cached) 
{
  if (!cached) 
  {
    //check if we already enqueued
    if (A_query_dir.done)
    {
      i2c_enqueue(&A_query_dir); 
    }

    if (A_query_state.done)
    {
      i2c_enqueue(&A_query_state); 
    }


    if (B_query_dir.done)
    {
      i2c_enqueue(&B_query_dir); 
    }

    if (B_query_state.done)
    {
      i2c_enqueue(&B_query_state); 
    }



    i2c_queue_flush(); //either way we need to wait 

    A_state.data = A_query_state.data; 
    B_state.data = B_query_state.data; 

    A_dir.data = A_query_dir.data; 
    B_dir.data = B_query_dir.data; 
  }

  if (!value) 
  {
    return 0; 
  }

  value->sbc = !(B_dir.data & ( 1 << I2C_EXPANDER_SBC_BIT));
  value->radiant = !(B_dir.data & ( 1 << I2C_EXPANDER_5V_1_BIT));
  value->lt = !(B_dir.data & ( 1 << I2C_EXPANDER_5V_2_BIT));

  value->surface_amps =0; 
  for (int i = 0; i < 6; i++) 
  {
    if (!(A_dir.data & ( 1 << surf_amp_map[i])))
      value->surface_amps |= (1 << i); 
  }

  value->dh_amps = 0;
  for (int i = 0; i < 3; i++) 
  {
    if (!(B_dir.data & ( 1 << dh_amp_map[i])))
      value->dh_amps |= (1 << i); 
  }

#ifdef _RNO_G_REV_D
  value->j29 = 0;
  value->ext_bus = 0; 
#else
  value->j29 = !(A_dir.data & (1 << I2C_EXPANDER_J29_BIT));
  value->ext_bus = !(A_dir.data & (1 << I2C_EXPANDER_EXT_BUS_BIT));
#endif

  return 0; 
}


int get_gpio_expander_fault_state(i2c_gpio_expander_t * faults) 
{

  //check if we already enqueued
  if (C.done)
  {
    i2c_enqueue(&C); 
  }

  if (D.done)
  {
    i2c_enqueue(&D); 
  }

  i2c_queue_flush(); //either way we need to wait 


  faults->sbc =!!( D.data & ( 1 << I2C_EXPANDER_SBC_BIT));
  faults->radiant = !!( D.data & ( 1 << I2C_EXPANDER_5V_1_BIT));
  faults->lt = !!(D.data & ( 1 << I2C_EXPANDER_5V_2_BIT));


  faults->surface_amps =0; 
  for (int i = 0; i < 6; i++) 
  {
    if (C.data & ( 1 << surf_amp_map[i]))
      faults->surface_amps |= (1 << i); 
  }

  faults->dh_amps = 0;
  for (int i = 0; i < 3; i++) 
  {
    if (D.data & ( 1 << dh_amp_map[i]))
      faults->dh_amps |= (1 << i); 
  }

  return 0; 
}

