#include "include/rno-g-control.h" 
#include "application/commands.h" 
#include "shared/spi_flash.h" 
#include "shared/config_block.h" 
#include "application/report.h" 
#include "application/mode.h" 




int commands_process()
{
  ; //nothing to do here... for now 
  return 0; 
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
      {
        return -1; 
      }
      const rno_g_cmd_report_t * cmd = (rno_g_cmd_report_t*) payload; 
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
    case RNO_G_CMD_LTE_STATS: 
    {
      if (payload_len != RNO_G_CMD_LTE_STATS) 
      {
        return -1; 
      }
 
      const rno_g_cmd_lte_stats_t * cmd = (rno_g_cmd_lte_stats_t*) payload; 
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

    case RNO_G_CMD_LORA_STATS: 
    {
      if (payload_len != RNO_G_CMD_LORA_STATS_SIZE)
      {
        return -1; 
      }
      const rno_g_cmd_lora_stats_t * cmd = (rno_g_cmd_lora_stats_t*) payload; 
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

    default:
      return -1; 
  }

  return 0; 
}


