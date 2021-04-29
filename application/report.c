#include "application/report.h" 
#include "application/time.h" 
#include "application/gpio_expander.h" 
#include "application/lowpower.h" 
#include "application/lte.h" 
#include "application/sbc.h" 
#include "application/monitors.h" 
#include "application/mode.h" 
#include "application/power.h" 
#include "shared/spi_flash.h"
#include "config/config.h" 

static rno_g_report_t report; 

void report_schedule(int navg) 
{
   power_monitor_schedule(); 
   if (!navg) navg=10; 
   monitor_fill(&report.analog_monitor,navg); 
}

void report_process() 
{
  static int report_ticks =0; 

  /// See if we need to do anything
  switch (report_ticks & ABOUT_10_SECONDS)
  {
    case 0: 
      if (report_ticks > 0)
      {
        power_monitor_schedule(); 
      }
      monitor_fill(&report.analog_monitor,20); 
      break; 
    case (300/DELAY_MS): 
      power_monitor_fill(&report.power_monitor); 
    default: 
      break; 
  }


  report_ticks++;
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
