#include "application/report.h" 
#include "application/time.h" 
#include "application/gpio_expander.h" 
#include "application/lowpower.h" 
#include "application/lte.h" 
#include "application/sbc.h" 
#include "lorawan/lorawan.h" 
#include "application/monitors.h" 
#include "application/mode.h" 
#include "application/power.h" 
#include "shared/spi_flash.h"
#include "config/config.h" 
#include <limits.h> 

static volatile rno_g_report_t report; 

static int report_scheduled_navg = 0; 
void report_schedule(int navg) 
{
  report_scheduled_navg = navg; 
}


const rno_g_report_t * report_process(int up, uint32_t * extrawake) 
{
  static uint32_t report_ticks =0; 
  static int last_report = INT_MIN;
  static uint32_t next_power_monitor_fill = 0; 
  static uint32_t power_mon_scheduled = 0; 
  
  const rno_g_report_t * ret = 0;

  /// See if we need to do anything
  

  int interval = low_power_mode ? config_block()->app_cfg.report_interval_low_power_mode : config_block()->app_cfg.report_interval; 
  if (interval < 10) interval = 10; //rate limit! 

  if (report_scheduled_navg ||   (up >= last_report + interval  &&  up > 5 && (!low_power_mode  || up >=60))  )   // wait until at least 5 seconds in to make a report , 60 seconds if in low power mode (to give chance to connect to lroa) 
  {
    low_power_mon_on(); 
    if (report_ticks > 0)
    {
        power_monitor_schedule(); 
    }
    monitor_fill(&report.analog_monitor,report_scheduled_navg ?: 20); 

    report_scheduled_navg = 0; 

    last_report = up; 
    int nticks = 300/DELAY_MS; 
    next_power_monitor_fill = report_ticks+nticks;
    power_mon_scheduled = 1; 
    if (extrawake) *extrawake+=nticks+2; 
  }

  if (power_mon_scheduled && report_ticks >= next_power_monitor_fill) 
  {
    low_power_mon_on(); //in case it got turned off since the step 
    power_monitor_fill(&report.power_monitor); 
    power_mon_scheduled = 0; 
    low_power_mon_off(); 

    ret = report_get(); 
    if (lorawan_state()  == LORAWAN_READY) 
    {
      lorawan_tx_copy(RNO_G_REPORT_SIZE ,RNO_G_MSG_REPORT , (uint8_t*) ret,  0);
      //Send twice, to improve chance we get it (because... confirmed doesn't work with the way our buffer works yet!) 
      if (low_power_mode) 
      {
        lorawan_tx_copy(RNO_G_REPORT_SIZE ,RNO_G_MSG_REPORT , (uint8_t*) ret,  0);
      }
    }
  }

  report_ticks++;
  return ret; 
}

const rno_g_report_t * report_get() 
{
  report.when = get_time(); 
  i2c_gpio_expander_t exp; 
  get_gpio_expander_state(&exp,1); 
  report.power_state.low_power_mode = low_power_mode; 
  report.power_state.sbc_power = exp.sbc;
  report.power_state.radiant_power = exp.radiant;
  report.power_state.lowthresh_power = exp.lt;
  report.power_state.lte_power = lte_get_state() != LTE_OFF; 
  report.power_state.dh_amp_power = exp.dh_amps; 
  report.power_state.surf_amp_power = exp.surface_amps; 
  report.mode = mode_query(); 
  report.lte_state = lte_get_state(); 
  report.sbc_state = sbc_get_state(); 
  report.sbc_boot_mode = config_block()->app_cfg.sbc_boot_mode; 
  return &report; 
}
