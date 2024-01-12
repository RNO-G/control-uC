#include "include/rno-g-control.h" 
#include "application/commands.h" 
#include "shared/spi_flash.h" 
#include "shared/config_block.h" 
#include "application/report.h" 
#include "application/heater.h" 
#include "application/mode.h" 
#include "lorawan/lorawan.h"
#include "application/i2cbus.h"
#include "application/i2cbusmux.h"
#include "application/lte.h"
#include "shared/printf.h" 
#include "application/reset.h" 
#include "hal_delay.h"




int commands_process()
{
  ; //nothing to do here... for now 
  return 0; 
}

static int handle_cmd_report(rno_g_cmd_report_t* cmd) 
{
  if (cmd->normal_interval) 
  {
    config_block()->app_cfg.report_interval = cmd->normal_interval;
  }
  else
  {
    config_block()->app_cfg.report_interval = default_app_config()->report_interval; 
  }

  if (cmd->low_power_interval) 
  {
    config_block()->app_cfg.report_interval_low_power_mode = cmd->low_power_interval;
  }
  else
  {
    config_block()->app_cfg.report_interval_low_power_mode = default_app_config()->report_interval_low_power_mode; 
  }

  config_block_sync(); 
  return 0;
}

static int handle_cmd_lte_stats(rno_g_cmd_lte_stats_t *cmd) 
{
  if (cmd->interval) 
  {
    config_block()->app_cfg.lte_stats_interval = cmd->interval;
  }
  else
  {
    config_block()->app_cfg.lte_stats_interval = default_app_config()->lte_stats_interval; 
  }
  config_block_sync(); 
  return 0;
}

static int handle_cmd_lora_stats(rno_g_cmd_lora_stats_t *cmd) 
{
  if (cmd->interval) 
  {
    config_block()->app_cfg.lora_stats_interval = cmd->interval;
  }
  else
  {
    config_block()->app_cfg.lora_stats_interval = default_app_config()->lora_stats_interval; 
  }

  if (cmd->low_power_interval) 
  {
    config_block()->app_cfg.lora_stats_interval_low_power_mode = cmd->low_power_interval;
  }
  else
  {
    config_block()->app_cfg.lora_stats_interval_low_power_mode = default_app_config()->lora_stats_interval_low_power_mode; 
  }

  config_block_sync(); 
  return 0;
}


static int handle_cmd_lora_timesync(rno_g_cmd_lora_timesync_t * cmd) 
{
  if (cmd->interval > 0) 
  {
    config_block()->app_cfg.timesync_interval = cmd->interval; 
    config_block_sync(); 
    return lorawan_sched_timesync();// sched now
  }
  else if (!cmd->interval)  // just once
  {
    return lorawan_sched_timesync();
  }
  return -1; 
}


static int handle_cmd_gps_secs_offset(rno_g_cmd_set_gps_secs_offset_t * cmd) 
{

    config_block()->app_cfg.gps_offset = cmd->offset; 
    config_block_sync(); 
    return 0; 
}


static int handle_cmd_set_battery_thresholds(rno_g_cmd_battery_thresholds_t * cmd) 
{
  if (cmd->turnoff_voltage > 0 ) 
  {
    config_block()->app_cfg.turnoff_voltage = cmd->turnoff_voltage; 
  }

  if (cmd->turnon_voltage > 0)  
  {
    config_block()->app_cfg.turnon_voltage = cmd->turnon_voltage; 

  }
  config_block_sync(); 
  return 0; 
}

static int handle_cmd_sbc(rno_g_cmd_sbc_t * cmd) 
{

  //not implemented yet 
  (void) cmd; 
  return -1; 

}

static int handle_cmd_heater(rno_g_cmd_heater_t * cmd) 
{
  if (cmd->heater & 1) return heater_on(); 
  else return heater_off(); 
}

static int handle_cmd_reset(rno_g_cmd_reset_t * cmd) 
{
  switch (cmd->what) 
  {
    case RNO_G_RESET_MICRO:
      printf("#LORA RESET INITIATED (TARGET=%d)", cmd->opt.micro.boot_opt); 
      delay_ms(10); 
      reset(cmd->opt.micro.boot_opt); 
      return 0; //yeah we won't get here
    case RNO_G_RESET_I2C:
      if (!cmd->opt.i2c.unstick)
#ifndef _RNO_G_REV_D
        return i2c_busmux_reset();
#else
        cmd->opt.i2c.unstick = 10; 
#endif
        return i2c_unstick(cmd->opt.i2c.unstick); 
    case RNO_G_RESET_LORA:
      return lorawan_reset(); 
    case RNO_G_RESET_LTE: 
      return lte_reset(cmd->opt.lte.reset_type); 
    default: 
      return -1; 

  }


}

int commands_put(uint8_t  opcode, uint8_t payload_len, const uint8_t * payload) 
{

  switch (opcode) 
  {
    case RNO_G_CMD_SET_MODE: 
    {
      if (payload_len != RNO_G_CMD_SET_MODE_SIZE) 
      {
        return -1; 
      }
      uint8_t mode = *payload; 
      if (mode >= RNO_G_NOT_A_MODE) 
      {
        //invalid 
        return -1; 
      }
      return mode_set((rno_g_mode_t) mode); 
    }
    case RNO_G_CMD_REPORT: 
    {
      if (payload_len != RNO_G_CMD_REPORT_SIZE)
        return -1; 
      return handle_cmd_report((rno_g_cmd_report_t*) payload); 
    }
    case RNO_G_CMD_LTE_STATS: 
    {
      if (payload_len != RNO_G_CMD_LTE_STATS) 
        return -1; 
 
      return handle_cmd_lte_stats((rno_g_cmd_lte_stats_t*) payload); 
    }

    case RNO_G_CMD_LORA_STATS: 
    {
      if (payload_len != RNO_G_CMD_LORA_STATS_SIZE)
        return -1; 

      return handle_cmd_lora_stats((rno_g_cmd_lora_stats_t*) payload); 
    }

    case RNO_G_CMD_LORA_TIMESYNC: 
    {
      if (payload_len != RNO_G_CMD_LORA_TIMESYNC_SIZE) 
      {
        return -1; 
      }
      return handle_cmd_lora_timesync((rno_g_cmd_lora_timesync_t*) payload); 
    }

    case RNO_G_CMD_SET_GPS_SECS_OFFSET: 
    {
      if (payload_len != RNO_G_CMD_SET_GPS_SECS_OFFSET_SIZE) 
      {
        return -1; 
      }
      return handle_cmd_gps_secs_offset((rno_g_cmd_set_gps_secs_offset_t*) payload); 
    }


    case RNO_G_CMD_SET_BATTERY_THRESHOLDS: 
    {
      if (payload_len != RNO_G_CMD_SET_BATTERY_THRESHOLDS_SIZE) 
      {
        return -1; 
      }
      return handle_cmd_set_battery_thresholds((rno_g_cmd_battery_thresholds_t*) payload); 
    }

    case RNO_G_CMD_SBC: 
    {
      if (payload_len != RNO_G_CMD_SBC_SIZE) 
      {
        return -1; 
      }
      return handle_cmd_sbc((rno_g_cmd_sbc_t*) payload); 
    }

    case RNO_G_CMD_RESET: 
    {
      if (payload_len != RNO_G_CMD_RESET_SIZE) 
      {
        return -1; 
      }
      return handle_cmd_reset((rno_g_cmd_reset_t*) payload); 
    }

    case RNO_G_CMD_HEATER: 
    {
      if (payload_len != RNO_G_CMD_HEATER_SIZE)
      {
        return -1; 
      }

      return handle_cmd_heater((rno_g_cmd_heater_t*) payload); 
    }

    default:
      return -1; 
  }

  return 0; 
}


