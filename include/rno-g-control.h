#ifndef _RNO_G_CONTROL_H
#define _RNO_G_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

/** Structs describing state, commands etc. 
 *
 * this will be used both on and off the MCU, so it needs to only depend on standard stuff.
 *
 * Only targetting gcc on little-endian arm-none-eabi (newlib), armv7l (Linux, glibc), and x86x64 (Linux, glibc). 
 *
 */

#include <stdint.h> 


/** The boot mode of the SBC 
 *
 *  This will affect next on-off cycle. 
 *
 **/ 
typedef enum sbc_boot_mode
{
  SBC_BOOT_INTERNAL = 0,
  SBC_BOOT_SDCARD = 1
}sbc_boot_mode_t; 

#define SBC_BOOT_MODE_STR(m) {"INTERNAL","SDCARD"}[m] 


/** The overarching mode of the station */ 
typedef enum rno_g_station_mode 
{
  RNO_G_INIT = 0,  
  RNO_G_NORMAL_MODE = 1,  // SBC on, LTE turned on (but can be cycled by SBC).  RADIANT / LT controlled by SBC 
  RNO_G_SBC_ONLY_MODE = 2, // only SBC on, LTE forced off  (can be used to forcibly remotely cycle LTE... the SBC won't be able to turn it back on though!)) 
  RNO_G_SBC_OFF_MODE = 3, // micro not in lower power mode, but SBC turned off. Can be used to forcibly cycle SBC. 
  RNO_G_LOW_POWER_MODE=4, //Low power mode. Everything but micro is off. 
  RNO_G_NOT_A_MODE = 5 // used for range check
} rno_g_mode_t; 

#define RNO_G_MODE_STR(m) (m < RNO_G_NOT_A_MODE ? {"INIT","NORMAL","SBC_ONLY","SBC_OFF","LOW_POWER"}[m] : "INVALID" )


/** TODO:
 * Check if this is binary compatible on all the targets */ 

typedef struct rno_g_power_state
{
  uint8_t low_power_mode : 1;  // low power mode supercedes basically everything else. this also includes the vicor 5V
  uint8_t sbc_power : 1;
  uint8_t lte_power : 1; 
  uint8_t radiant_power : 1; 
  uint8_t lowthresh_power : 1; 
  uint8_t dh_amp_power : 3; 
  uint8_t surf_amp_power : 6; 
} rno_g_power_state_t; 

#define STRBL(x) (x) ? "true" : "false"


#define RNO_G_POWER_STATE_JSON_FMT "{\"low_power_mode\": %s,\"sbc_power\":%s,"\
                                   "\"lte_power\": %s,\"radiant_power\":%s,"\
                                   "\"lowthresh_power\":%s,\"dh_amp_power\":[%s,%s,%s],"\
                                   "\"surf_amp_power:\":[%s,%s,%s,%s,%s,%s]}"; 

#define RNO_G_POWER_STATE_JSON_VALS(ps) \
  STRBL(ps.low_power_mode),\
  STRBL(ps.sbc_power),\
  STRBL(ps.lte_power),\
  STRBL(ps.radiant_power),\
  STRBL(ps.lowthresh_power),\
  STRBL(ps.dh_amp_power & 1),\
  STRBL(ps.dh_amp_power & 2),\
  STRBL(ps.dh_amp_power & 4),\
  STRBL(ps.surf_amp_power & 1),\
  STRBL(ps.surf_amp_power & 2),\
  STRBL(ps.surf_amp_power & 4),\
  STRBL(ps.surf_amp_power & 8),\
  STRBL(ps.surf_amp_power & 16),\
  STRBL(ps.surf_amp_power & 32)





/** voltages, temperatures, etc. recorded by the MCU ADC */ 
typedef struct rno_g_monitor
{
  uint32_t when; 
  int16_t temp_cC; 
  int16_t i_surf3v[6]; 
  int16_t i_down3v[3]; 
  int16_t i_sbc5v; 
  int16_t i_5v[2]; 
} rno_g_monitor_t;

#define RNO_G_MONITOR_JSON_FMT "{\"when\": %u, \"temp_C\": %f, \"I_surf3V_mA\":[%d,%d,%d,%d,%d,%d],"\
                               "\"I_down3V_mA\":[%d,%d,%d],\"I_sbc5V_mA\":%d,\"I_radiant_mA\":%d,\"I_lowthresh_mA\":%d}"

#define RNO_G_MONITOR_JSON_VALS(mon) \
  mon.when,mon.temp_cC/100.,\
  mon.i_surf3v[0],mon.i_surf3v[1],\
  mon.i_surf3v[2],mon.i_surf3v[3],\
  mon.i_surf3v[4],mon.i_surf3v[5],\
  mon.i_down3v[0],mon.i_down3v[1],mon.i_down3v[2]\
  mon.isbc5v,mon.i_5v[0],mon.i_5v[1] 


/** Monitoring from the power board */ 
typedef struct power_system_monitor
{
  uint32_t when_power;

  uint16_t PVv_cV; 
  uint16_t PVi_mA; 

  uint16_t BATv_cV; 
  uint16_t BATi_mA;  //

  uint32_t when_temp;

  int8_t local_T_C; //deci-celsius
  int8_t remote1_T_C; //deci-celsius
  int8_t remote2_T_C; //deci-celsius

  uint8_t local_T_sixteenth_C : 4; 
  uint8_t remote1_T_sixteenth_C : 4; 
  uint8_t remote2_T_sixteenth_C : 4; 

} rno_g_power_system_monitor_t;

#define RNO_G_POWER_SYSTEM_MONITOR_JSON_FMT "{\"when_power\":%u,\"PV_V\":%f,\"PV_I_mA\":%u,\"BAT_V\":%f,\"BAT_I_mA\":%u"\
                                            "\"when_temp\":%u,\"local_T_C\":%f,\"remote1_T_C\":%f:\"remote2_T_C\":%f}"

#define RNO_G_POWER_SYSTEM_MONITOR_JSON_VALS(pm) \
 pm.when_power,\
 pm.PVv_cV/100.,\
 pm.PVi_mA.,\
 pm.BATv_cV/100.,\
 pm.BATi_mA.,\
 pm.when_temp.,\
 pm.local_T_c+local_T_sixteenth_C/16.,\
 pm.remote1_T_c+remote1_T_sixteenth_C/16.,\
 pm.remote2_T_c+remote2_T_sixteenth_C/16.


typedef enum lte_state
{
  LTE_INIT, 
  LTE_OFF, 
  LTE_TURNING_ON,
  LTE_ON,
  LTE_TURNING_OFF
} lte_state_t; 

#define LTE_STATE_STR(state)  {"LTE_INIT","LTE_OFF","LTE_TURNING_ON","LTE_TURNING_OFF"}[state] 

typedef enum sbc_state
{
  SBC_OFF, 
  SBC_ON,
  SBC_TURNING_ON, 
  SBC_TURNING_OFF
} sbc_state_t; 


#define SBC_STATE_STR(state)  {"SBC_OFF","SBC_ON","SBC_TURNING_ON","SBC_TURNING_OFF"}[state] 


/** These are the messages you can get from the LoRaWAN */ 
typedef uint8_t rno_g_msg_type_t;  // note that 0 and 224-255 are reserved for other purposes since we're abusing the LoRaWAN port field 
enum rno_g_msg_type
{
  RNO_G_MSG_REPORT = 1, 
  RNO_G_MSG_LTE_STATS = 2, 
  RNO_G_MSG_LORA_STATS = 3, 
  RNO_G_MSG_SBC = 4 
}; 

/** TODO, shrink this so it can fit in DR2 (53 bytes). Right now is 68 bytes 
 *
 *  Bridge can distinguish based on payload size as long as we keep the old struct def alive somehow. 
 * */ 

typedef struct rno_g_report
{
  uint32_t when; 
  uint8_t mode;  
  uint8_t lte_state;
  uint8_t sbc_state; 
  uint8_t sbc_boot_mode;
  rno_g_monitor_t analog_monitor; 
  rno_g_power_system_monitor_t power_monitor; 
  rno_g_power_state_t power_state; 
}rno_g_report_t; 


#define RNO_G_REPORT_JSON_FMT "{\"when\":%u,\"mode\":\"%s\",\"lte_state\":\"%s\",\"sbc_state\":\"%s\,\"sbc_boot_mode\":\"%s\""\
                              "\"analog\":" RNO_G_MONITOR_JSON_FMT\
                              "\"power_monitor\":" RNO_G_POWER_SYSTEM_MONITOR_JSON_FMT\
                              "\"power_state\":" RNO_G_POWER_SYSTEM_MONITOR_JSON_FMT "}" 

#define RNO_G_REPORT_JSON_VALS(r) \
  r.when, RNO_G_MODE_STR(r.mode), LTE_STATE_STR(r.lte_state), SBC_STATE_STR(r.sbc_state), SBC_BOOT_MODE_STR(r.sbc_boot_mode),\
  RNO_G_MONITOR_JSON_VALS(r.analog_monitor), RNO_G_POWER_SYSTEM_MONITOR_JSON_VALS(r.power_monitor), RNO_G_POWER_STATE_JSON_VALS(r.power_state)
  


typedef struct rno_g_lte_stats
{
  uint32_t when;
  int16_t mcc;
  int16_t mnc; 
  uint16_t earfcn; 
  int8_t rsrp; 
  int8_t rssi; 
  uint8_t neg_rsrq_x10; 
  uint8_t tx_power; 
  uint8_t band : 6; 
  uint8_t service_domain : 2; 
  uint8_t parsed_ok; 
}rno_g_lte_stats_t; 


typedef struct rno_g_lora_stats
{
  uint32_t when;
  uint32_t uptime;
  uint32_t rx; 
  uint32_t tx; 
  uint32_t tx_dropped;
  uint32_t rx_dropped;
  int last_recv; 
  int last_sent; 
  int last_check; 
  int join_time; 
  short rssi; 
  short snr; 
} rno_g_lora_stats_t; 


typedef struct rno_g_sbc_msg
{
  uint32_t when; 
  char msg[40]; 
} rno_g_sbc_msg_t; 

enum rno_g_msg_size
{
  RNO_G_REPORT_SIZE = sizeof(rno_g_report_t),
  RNO_G_LTE_STATS_SIZE = sizeof(rno_g_lte_stats_t),
  RNO_G_LORA_STATS_SIZE = sizeof(rno_g_lora_stats_t), 
  RNO_G_SBC_MSG_SIZE = sizeof(rno_g_sbc_msg_t)
}; 





typedef uint8_t rno_g_cmd_type_t;   // note that 0 and 224-255 are reserved for other purposes since we're abusing the LoRaWAN port field

enum rno_g_cmd_type
{
  RNO_G_CMD_TOO_SMALL = 0, // 
  RNO_G_CMD_SET_MODE = 1, 
  RNO_G_CMD_REPORT=2, 
  RNO_G_CMD_LTE_STATS=3,
  RNO_G_CMD_LORA_STATS=4,
  RNO_G_CMD_LORA_TIMESYNC=5,
  RNO_G_CMD_SET_GPS_SECS_OFFSET=6, 
  RNO_G_CMD_SET_BATTERY_THRESHOLDS=7, 
  RNO_G_CMD_SBC=8, 
  RNO_G_CMD_TOO_LARGE = 9
} ; 

/* Sets the running mode */ 
typedef struct rno_g_cmd_set_mode
{
  rno_g_mode_t set; 
} rno_g_cmd_set_mode_t; 

typedef struct rno_g_cmd_report
{
  uint16_t normal_interval;  // in seconds. 0 is treated as default
  uint16_t low_power_interval;  // in seconds. 0 is treated as default 
} rno_g_cmd_report_t; 

typedef struct rno_g_cmd_lte_stats
{
  uint16_t interval; //in seconds, 0 treated as dfeault
} rno_g_cmd_lte_stats_t; 


typedef struct rno_g_cmd_lora_stats
{
  uint16_t interval; //in seconds, 0 treated as default
  uint16_t low_power_interval; //in seconds, 0 treated a sdefault
} rno_g_cmd_lora_stats_t; 


typedef struct rno_g_cmd_lora_timesync
{
  uint16_t interval; //0 = once, now, otherwise in seconds
} rno_g_cmd_lora_timesync_t; 

typedef struct rno_g_cmd_set_gps_secs_offset
{
  int8_t offset; 
} rno_g_cmd_set_gps_secs_offset_t; 

typedef struct rno_g_cmd_battery_thresholds
{
  float turnoff_voltage;
  float turnon_voltage; 
} rno_g_cmd_battery_thresholds_t; 

typedef struct rno_g_cmd_sbc
{
  char cmd[40]; 
  uint8_t max_response_len; 
  uint8_t max_history_len; 
} rno_g_cmd_sbc_t; 


enum rno_g_cmd_size
{
  RNO_G_CMD_SET_MODE_SIZE = sizeof(rno_g_cmd_set_mode_t), 
  RNO_G_CMD_REPORT_SIZE = sizeof(rno_g_cmd_report_t), 
  RNO_G_CMD_LTE_STATS_SIZE = sizeof(rno_g_cmd_lte_stats_t), 
  RNO_G_CMD_LORA_STATS_SIZE = sizeof(rno_g_cmd_lora_stats_t), 
  RNO_G_CMD_LORA_TIMESYNC_SIZE = sizeof(rno_g_cmd_lora_timesync_t),  
  RNO_G_CMD_SET_GPS_SECS_OFFSET_SIZE = sizeof(rno_g_cmd_set_gps_secs_offset_t), 
  RNO_G_CMD_SET_BATTERY_THRESHOLDS_SIZE = sizeof(rno_g_cmd_battery_thresholds_t), 
  RNO_G_CMD_SBC_SIZE = sizeof(rno_g_cmd_sbc_t)
}; 


#ifdef __cplusplus
}
#endif 

#endif

