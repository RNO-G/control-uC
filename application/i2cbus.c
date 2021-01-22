#include "application/i2cbus.h" 
#include "shared/driver_init.h" 
#include "hal_i2c_m_async.h" 
#include "application/gpio_expander.h" 

#include "hal_atomic.h" 


static void init_i2c_devices()
{
  //make sure the gpio expander state is correct
  get_gpio_expander_state(0,0); 
}


#if USE_SYNCHRONOUS_I2C
#include "hal_i2c_m_sync.h" 

void i2c_bus_init()
{
  i2c_m_sync_enable(&I2C); 
  init_i2c_devices(); 
}
 

int i2c_queue_size() { return 0; } 

int i2c_enqueue(i2c_task_t * task) 
{
  task->done = 0; 
  i2c_m_sync_set_slaveaddr(&I2C, task->addr, I2C_M_SEVEN); 

  int ret = 0; 

  if (task->write)
  {
    uint8_t buf[2] = {task->reg, task->data}; 
    struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_STOP, .buffer = buf, .len =2 }; 
    ret = i2c_m_sync_transfer(&I2C, &msg); 

  }
  else
  {
    ret = i2c_m_sync_cmd_read(&I2C, task->reg, &task->data, 1); 
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
  if (task->write) 
  { 
    uint8_t buf[2] = {task->reg, task->data}; 
    struct _i2c_m_msg msg = {.addr = task->addr, .flags = I2C_M_STOP, .buffer = buf, .len =2 }; 
    return i2c_m_async_transfer( &I2C, &msg); 
  }
  else
  {
    //well crap, we need to write the register first. 
    struct _i2c_m_msg msg = {.addr = task->addr, .flags = 0, .buffer = &task->reg, .len =1 }; 
    return i2c_m_async_transfer( &I2C, &msg); 
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



