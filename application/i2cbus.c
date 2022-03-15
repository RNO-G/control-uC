#include "application/i2cbus.h" 
#include "application/i2cbusmux.h" 
#include "shared/driver_init.h" 
#include "hal_i2c_m_async.h" 
#include "application/gpio_expander.h" 
#include "shared/printf.h" 
#include <string.h>
#include "hal_atomic.h" 


static void init_i2c_devices()
{

#ifndef _RNO_G_REV_D
  i2c_busmux_init(); 
#endif

  //make sure the gpio expander state is correct
  gpio_expander_init(); 
}


#if USE_SYNCHRONOUS_I2C
#include "hal_i2c_m_sync.h" 

void i2c_bus_init()
{
	I2C_init();
  i2c_m_sync_enable(&I2C); 
  init_i2c_devices(); 
}
 
void i2c_bus_deinit()
{
  i2c_m_sync_disable(&I2C); 
}

int i2c_queue_size() { return 0; } 

int i2c_enqueue(i2c_task_t * task) 
{
  if (!task->addr) 
  {
    task->done = -1; 
    return -1; 
  }
  task->done = 0; 
  i2c_m_sync_set_slaveaddr(&I2C, task->addr, I2C_M_SEVEN); 

  int ret = 0; 
  int regless = task->flags & I2CTSK_REG_LESS; 


  if (task->write)
  {
    uint8_t buf[2] = {task->reg, task->data}; 
    struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_STOP, .buffer = buf + regless,  .len =2-regless }; 
    ret = i2c_m_sync_transfer(&I2C, &msg); 

  }
  else
  {
    if (regless) 
    {
      struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_STOP | I2C_M_RD, .buffer = &task->data, .len=1}; 
      ret = i2c_m_sync_transfer(&I2C, &msg); 
    }
    else 
    {
      ret = i2c_m_sync_cmd_read(&I2C, task->reg, &task->data, 1); 
    }

  }

  if (ret < 0) i2c_m_sync_send_stop(&I2C); //this should only be necessary on error, I think? 
  task->done =  ret < 0 ? ret : 1; 
  return ret; 
}


void i2c_queue_flush() { ; } 

#else

volatile static uint8_t i2c_busy; 
volatile static uint8_t tasks_queued; 
volatile static uint8_t tasks_done; 
static i2c_task_t* tasks[I2C_TASK_BUFFER_SIZE]; 

static int sched_next_task(int); 

void i2c_success_cb(struct i2c_m_async_desc * const i2c) 
{
  (void) i2c; 

  i2c_task_t * task = tasks[tasks_done % I2C_TASK_BUFFER_SIZE]; 

  //do we need to send the actual read? 
  if ( !(i2c->device.service.msg.flags & I2C_M_RD)  &&  !task->write) 
  {
    struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_STOP | I2C_M_RD, .buffer = &task->data, .len =1 }; 
    i2c_m_async_transfer( &I2C, &msg); 
    return; 
  }


  i2c_busy=0;
  task->done = 1; 
  tasks_done++; 
  sched_next_task(0); 
}

void i2c_error_cb(struct i2c_m_async_desc * const i2c, int32_t error) 
{
  tasks[tasks_done % I2C_TASK_BUFFER_SIZE]->done = error; 
  tasks_done++; 
  i2c_m_async_send_stop(&I2C); 
  i2c_busy=0;
  sched_next_task(0); 
}


void i2c_bus_init()
{
  i2c_m_async_enable(&I2C); 
  i2c_m_async_register_callback(&I2C, I2C_M_ASYNC_TX_COMPLETE, (FUNC_PTR) &i2c_success_cb);
  i2c_m_async_register_callback(&I2C, I2C_M_ASYNC_RX_COMPLETE, (FUNC_PTR) &i2c_success_cb);
  i2c_m_async_register_callback(&I2C, I2C_M_ASYNC_ERROR, (FUNC_PTR) &i2c_error_cb);
 
  init_i2c_devices(); 
}

static void i2c_bus_deinit()
{
  i2c_m_async_disable(&I2C); 
}



static int sched_next_task(int check_busy)
{
  int should_return = 0; 
  CRITICAL_SECTION_ENTER() 
  if (tasks_queued == tasks_done) 
  {
  //nothing to do! 
    i2c_busy = 0; 
    should_return = 1; 
  }
  if (check_busy && i2c_busy)
  {
    should_return = 1; 
  }
  i2c_busy = 1; 
  CRITICAL_SECTION_LEAVE(); 
  if (should_return) return 0; 
  
  i2c_task_t * task = tasks[tasks_done % I2C_TASK_BUFFER_SIZE]; 
  i2c_m_async_set_slaveaddr(&I2C, task->addr, I2C_M_SEVEN); 
  task->done = 0; 
  int regless = task->flags & I2CTSK_REG_LESS; 
  if (task->write) 
  { 
    uint8_t buf[2] = {task->reg, task->data}; 
    struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_STOP, .buffer = buf+regless, .len =2 -regless}; 
    return i2c_m_async_transfer( &I2C, &msg); 
  }
  else
  {
    //well crap, we need to write the register first. 
    if (!regless) 
    {
      struct _i2c_m_msg msg = {.addr = task->addr, .flags = 0, .buffer = &task->reg, .len =1 }; 
      return i2c_m_async_transfer( &I2C, &msg); 
    }
    else
    {
      struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_RD | I2C_M_STOP , .buffer = &task->data, .len =1 }; 
      return i2c_m_async_transfer( &I2C, &msg); 

    }
  }

}

int i2c_queue_size() 
{
  return tasks_queued - tasks_done; 

}

int i2c_enqueue(i2c_task_t * task) 
{
  if (i2c_queue_size() > I2C_TASK_BUFFER_SIZE) 
  {
    task->done = -1; 
    return -1; 
  }

  tasks[tasks_queued % I2C_TASK_BUFFER_SIZE] = task; 
  tasks_queued++; 

  //is the i2c queue busy? if not, just do it
  if (!i2c_busy) 
  {
    return sched_next_task(1); 
  }

  return 0; 

}

void i2c_queue_flush() 
{
  while (i2c_queue_size()) 
  {
    ;

  }
}

#endif

static uint8_t internal_bitset[16]; 

int i2c_detect(uint8_t start_addr, uint8_t end_addr, uint8_t *bitset) 
{

  uint8_t start = start_addr < 0x08 ? 0x08 : start_addr; 
  uint8_t end = end_addr >= 0x78 ? 0x77 : end_addr; 

  if (bitset) memset(bitset,0,16); 
  else memset(internal_bitset,0,16); 

  uint8_t dummy = 0; 
  for (uint8_t addr = start ; addr <= end; addr++) 
  {
    i2c_m_sync_set_slaveaddr(&I2C, addr, I2C_M_SEVEN); 
    int ret = i2c_m_sync_cmd_read(&I2C,0,&dummy,1); 
    if (!ret) 
    { 
      if (bitset) 
      {
        bitset[addr >> 3] |= (1 << (addr &0x7)); 
      }
      else 
      {
        internal_bitset[addr >> 3] |= (1 << (addr & 0x7)); 
        printf("#I2C-DETECT %x  (val(0)=%x)\n", addr, dummy); 
      }
    }
  }
  return 0; 
}


int i2c_unstick(int ncycles) 
{
  i2c_bus_deinit(); 

  int nones = 0; 
	gpio_set_pin_direction(I2C_SDA, GPIO_DIRECTION_IN);
	gpio_set_pin_direction(I2C_SCL, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(I2C_SCL, 0);

  for (int i = 0; i < ncycles; i++) 
  {
    delay_us(10); 
    gpio_set_pin_level(I2C_SCL, 1);
    delay_us(5); 
    nones += gpio_get_pin_level(I2C_SDA); 
    delay_us(5); 
    gpio_set_pin_level(I2C_SCL, 0);
  }


  i2c_bus_init(); 
  return nones; 
}



