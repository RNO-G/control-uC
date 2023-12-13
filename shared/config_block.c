#include "shared/config_block.h" 
#include <math.h> 


#ifndef INITIAL_STATION_NUMBER
#define INITIAL_STATION_NUMBER 999 
#endif 

#define XSTR(X) STR(X) 
#define STR(X) #X 
#pragma message ("Initial Station Number is " XSTR(INITIAL_STATION_NUMBER)) 


void default_init_boot_cfg(bootloader_cfg_t * bc) 
{

  bootloader_cfg_t dflt =  
  {
    .n_resets_before_reflash = 0, 
    .recovery_priority_list = {1,2,3,4}
  };
  *bc = dflt; 
}

static  application_cfg_t dflt_ac = 
   {
     .wanted_state = RNO_G_NORMAL_MODE, 
     .station_number = INITIAL_STATION_NUMBER, 
     .gps_offset = 18,
     .sbc_boot_mode = SBC_BOOT_INTERNAL , 
     .report_interval = 60, 
     .report_interval_low_power_mode = 3600, 
     .lte_stats_interval = 60, 
     .lora_stats_interval = 60, 
     .lora_stats_interval_low_power_mode = 600, 
     .timesync_interval = 1800,
#ifdef _RNO_REV_D
     .turnon_voltage = 0, 
     .turnoff_voltage = 0 

#else
     .turnon_voltage = 0, 
     .turnoff_voltage = 0 
#endif 
   }; 


void default_init_app_cfg(application_cfg_t * ac) 
{
  *ac = dflt_ac; 
}

int verify_app_cfg(application_cfg_t* ac) 
{
  int changed = 0;
  if (ac->wanted_state >= RNO_G_NOT_A_MODE) 
  {
    ac->wanted_state = dflt_ac.wanted_state;
    changed++; 
  }
  if (ac->sbc_boot_mode > SBC_BOOT_SDCARD)
  {
    ac->sbc_boot_mode = dflt_ac.sbc_boot_mode; 
    changed++; 
  }
  if (ac->report_interval == -1)
  {
    ac->report_interval = dflt_ac.report_interval; 
    changed++; 
  }
  if (ac->report_interval_low_power_mode == -1)
  {
    ac->report_interval_low_power_mode = dflt_ac.report_interval_low_power_mode; 
    changed++;
  }
  if (ac->lte_stats_interval == -1)
  {
    ac->lte_stats_interval = dflt_ac.lte_stats_interval; 
    changed++; 
  }
  if (ac->lora_stats_interval == -1)
  {
    ac->lora_stats_interval = dflt_ac.lora_stats_interval; 
    changed++; 
  }
  if (ac->lora_stats_interval_low_power_mode == -1) 
  {
    ac->lora_stats_interval_low_power_mode = dflt_ac.lora_stats_interval_low_power_mode; 
    changed++; 
  }
  if (isnan(ac->turnon_voltage)) 
  {
    ac->turnon_voltage = dflt_ac.turnon_voltage; 
    changed++; 
  }
  if (isnan(ac->turnoff_voltage)) 
  {
    ac->turnoff_voltage = dflt_ac.turnoff_voltage; 
    changed++; 
  }
#ifndef _RNO_G_REV_D
  if (ac->turnon_voltage < 20) 
  {
    ac->turnon_voltage = 30; 
    changed++; 
  }
#endif
  if (ac->timesync_interval == -1) 
  {
    ac->timesync_interval = 1800; 
  }
  return changed; 
}

void default_init_block(config_block_t * block) 
{
  default_init_boot_cfg(&block->boot_cfg); 
  default_init_app_cfg(&block->app_cfg); 
}

const application_cfg_t * default_app_config() 
{
  return &dflt_ac; 
}

