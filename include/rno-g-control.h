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

#define SBC_BOOT_MODE_STR(m)  ( m == SBC_BOOT_INTERNAL ? "INTERNAL" : m == SBC_BOOT_SDCARD ? "SDCARD" : "INVALID" )


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

#define RNO_G_MODE_STR(m) ( m == RNO_G_INIT ? "INIT" : m == RNO_G_NORMAL_MODE ? "NORMAL" : m == RNO_G_SBC_ONLY_MODE ? "SBC_ONLY" : m ==RNO_G_SBC_OFF_MODE ? "SBC_OFF" : m==RNO_G_LOW_POWER_MODE ? "LOW_POWER" : "INVALID" )


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
  uint8_t j29_power : 1;  /* MIGHT BE GARBAGE FOR REV_D. Safe to extend due to padding, I think */
  uint8_t output_bus_enable : 1; /* MIGHT BE GARBAGE FOR REV_D. Safe to extend due to padding, I think */
} rno_g_power_state_t; 

#define STRBL(x) (x) ? "true" : "false"

#ifdef _RNO_G_REV_D
#define STRBL_OR_NA(x)  "N/A"
#else
#define STRBL_OR_NA(x) STRBL(x) 
#endif



#define RNO_G_POWER_STATE_JSON_FMT "{\"low_power_mode\": %s,\"sbc_power\":%s,"\
                                   "\"lte_power\": %s,\"radiant_power\":%s,"\
                                   "\"lowthresh_power\":%s,\"dh_amp_power\":[%s,%s,%s],"\
                                   "\"surf_amp_power:\":[%s,%s,%s,%s,%s,%s]," \
                                   "\"j29_power\": %s, \"output_bus_enable\": %s }"  

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
  STRBL(ps.surf_amp_power & 32),\
  STRBL_OR_NA(ps.j29_power),\
  STRBL_OR_NA(ps.output_bus_enable)



/** voltages, temperatures, etc. recorded by the MCU ADC 
 *  Disused with RevD. 
 *
 * */ 
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


/** Monitoring from the power board 
 * Disused with RevD. 
 * */ 
typedef struct power_system_monitor
{
  uint32_t when_power;

  uint16_t PVv_cV; 
  uint16_t PVi_mA; 

  uint16_t BATv_cV; 
  uint16_t BATi_mA;  //

  uint32_t when_temp;

  int8_t local_T_C; //celsius
  int8_t remote1_T_C; //celsius
  int8_t remote2_T_C; //celsius

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
  LTE_TURNING_OFF, 
  LTE_RESETTING
} lte_state_t; 

#define LTE_STATE_STR(s) \
  ( s == LTE_INIT  ? "INIT" : \
    s == LTE_OFF ? "OFF" : \
    s == LTE_TURNING_ON ? "TURNING_ON" : \
    s == LTE_ON ? "ON" : \
    s == LTE_TURNING_OFF ? "TURNING_OFF" : \
    s == LTE_RESETTING ? "RESETTING" :\
    "INVALID" )

typedef enum sbc_state
{
  SBC_OFF, 
  SBC_ON,
  SBC_TURNING_ON, 
  SBC_TURNING_OFF
} sbc_state_t; 


#define SBC_STATE_STR(s)   ( s == SBC_OFF ? "OFF" : s == SBC_ON ? "ON" : s == SBC_TURNING_ON ? "TURNING_ON" : s == SBC_TURNING_OFF ? "TURNING_OFF" : "INVALID" )


/** These are the messages you can get from the LoRaWAN */ 
typedef uint8_t rno_g_msg_type_t;  // note that 0 and 224-255 are reserved for other purposes since we're abusing the LoRaWAN port field 
enum rno_g_msg_type
{
  RNO_G_MSG_REPORT = 1, 
  RNO_G_MSG_LTE_STATS = 2, 
  RNO_G_MSG_LORA_STATS = 3, 
  RNO_G_MSG_SBC = 4,
  RNO_G_MSG_REPORT_V2 = 5, 
}; 

/** 
 *
 * This is the report class for RevD  
 * Superceded by report v2 in new firmware. 
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
  



/** More tightly packed version of report,
 * with more information, but not split up into structs.
 *
 * All currents in milliamps, all voltages in amps, but heed the _div suffixes which means that the value is divied by that much (and should be multiplied by that amount to get it back)!  
 **/ 
typedef struct rno_g_report_v2
{
  int when; 
  //4 bytes
  uint8_t mode : 3;
  uint8_t lte_state : 3;
  uint8_t sbc_state : 2;
  //5 bytes
  uint8_t sbc_boot_mode : 1; 
  int8_t analog_delta_when : 7;  // seconds difference between analog and when 
  //6 bytes 
  uint8_t i_sbc_div4;  // use 8 bits for SBC, divided by 4 so max is about 1 A with 4 mA resolution
  //7 bytes 
  uint8_t i_surf_div4[6]; // surface receivers, 8 bits, divided by 4 so max is about 1 A with 4 mA resolution
  // 13 bytes
  uint8_t i_dh_div4[3]; // downhole receivers, 8 bits, divided by 4 so max is about 1 A with 4 mA resolution
  //16 bytes
  uint16_t i_lt_div1p786 : 12; //  low threshold board, 12 bits,  mA conversion is 125/70, max is 7.3 A. 
  uint16_t i_radiant_div1p786 : 12; //  low threshold board, 12 bits,  mA conversion is 125/70, max is 7.3 A
  int8_t  V_radiant_div25; // 
  //20 bytes
  int8_t  V_lt_div25; // 
  int8_t  digi_delta_when; 
  int8_t power_delta_when; 
  int8_t temp_delta_when;
  //24 bytes 
  uint16_t i_pv_div2p5 : 12 ; //  RANGE TBD
  uint16_t V_pv_div25 : 12 ;  
  uint16_t i_batt_div1p25 : 12 ; //  
  uint16_t V_batt_div25 : 12 ; // 
  int16_t T_local_times16 : 12;   
  int16_t T_remote_1_times16 : 12;   
  int16_t T_remote_2_times16 : 12;   
  int16_t T_micro_times16 : 12;   
  //36 bytes
  rno_g_power_state_t power_state; 
  //38 bytes 
  int16_t V_5_div1p5 : 12; 
  uint8_t reserved : 4; 
  int8_t V_lte_div16; 
  int8_t V_33_div16; 
  //42 bytes, probably. At most
}rno_g_report_v2_t; 


#define RNO_G_REPORT_V2_JSON_FMT "{\"when\":%d,\"mode\":\"%s\",\"lte_state\":\"%s\",\"sbc_state\":\"%s\",\"sbc_boot_mode\":\"%s\", "\
                              "\"currents\": {\"sbc\": %d, \"surf\": [%d,%d,%d,%d,%d,%d]\", \"dh\": [%d,%d,%d], \"lt\": %g, \"radiant\": %g, \"batt\": %g, \"pv\": %g }, "\
                              "\"voltages\": {\"lt\": %d, \"radiant\": %d, \"5v\": %g, \"3.3v\": %d, \"lte\":  \"batt\": %d, \"pv\": %d }, "\
                              "\"temps\": {\"local\": %g, \"remote_1\": %g, \"remote_2\": %g, \"micro\": %g }, "\
                              "\"when_analog\": %d, \"when_digi\": $d, \"when_power\": %d, \"when_temp\": %d, "\
                              "\"power_state\":" RNO_G_POWER_STATE_JSON_FMT "}" 


#define RNO_G_REPORT_V2_JSON_VALS(r) \
  r->when, RNO_G_MODE_STR(r->mode), LTE_STATE_STR(r->lte_state), SBC_STATE_STR(r->sbc_state), SBC_BOOT_MODE_STR(r->sbc_boot_mode),\
  r->i_sbc_div4*4, r->i_surf_div4[0]*4, r->i_surf_div4[1]*4,r->i_surf_div4[2]*4, r->i_surf_div4[3]*4,r->i_surf_div4[4]*4, r->i_surf_div4[5]*4,\
  r->i_dh_div4[0]*4, r->i_dh_div4[1]*4,r->i_dh_div4[2]*4,\
  r->i_lt_div1p786 * (125/70.),r->i_radiant_div1p786 * (125/70.), r->i_batt_div1p25 * 1.25,r->i_pv_div2p5 * 2.5, \
  r->V_lt_div25*25, r->V_radiant_div25*25, r->V_5_div1p5 * 1.5, r->V_33_div16 * 15, r->V_lte_div16 * 16,  r->V_batt_div25*25, r->V_pv_div25*25,\
  r->T_local_times16/16., \
  r->T_remote_1_times16/16., \
  r->T_remote_2_times16/16., \
  r->T_micro_times16/16., \
  r->when + r->analog_delta_when, r->when + r->digi_delta_when, r->when + r->power_delta_when, r->when + r->temp_delta_when, \
  RNO_G_POWER_STATE_JSON_VALS(r->power_state)


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
  RNO_G_SBC_MSG_SIZE = sizeof(rno_g_sbc_msg_t),
  RNO_G_REPORT_V2_SIZE = sizeof(rno_g_report_v2_t),
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
  RNO_G_CMD_RESET=9, 
  RNO_G_CMD_TOO_LARGE = 10
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


typedef enum 
{
  LTE_SOFT_CYCLE = 0, 
  LTE_HARD_CYCLE = 1, 
  LTE_POWER_CYCLE = 2, 
  LTE_FACTORY_RESET = 3, 
  LTE_NOT_A_RESET 
} rno_g_lte_reset_type_t; 

typedef enum 
{
  BOOT_ROM=0, 
  BOOT_FLASH_SLOT_1 = 1, 
  BOOT_FLASH_SLOT_2 = 2, 
  BOOT_FLASH_SLOT_3 = 3, 
  BOOT_FLASH_SLOT_4 = 4, 
  BOOT_BOOTLOADER =10 
} rno_g_boot_option_t; 

typedef enum 
{
  SBC_RESET_OFF_ON, 
  SBC_RESET_RST
} rno_g_sbc_reset_type; 


// resets a subsystem. what tells what to reset, opt is an extra argument for some modes (should be safe to 0)
typedef struct rno_g_cmd_reset
{
  enum 
  {
    RNO_G_RESET_MICRO, 
    RNO_G_RESET_I2C, 
    RNO_G_RESET_LORA, 
    RNO_G_RESET_LTE
  } what; 

  union 
  {
    struct
    {
      rno_g_boot_option_t boot_opt; 
    } micro; 

    struct 
    {
      int unstick; //if 0, uses busmux, otherwise uses i2c_unstick. if rev_d and 0, same as 10. 
    } i2c; 

    struct 
    {
      rno_g_lte_reset_type_t reset_type; 
    } lte;
  } opt; 
} rno_g_cmd_reset_t; 

enum rno_g_cmd_size
{
  RNO_G_CMD_SET_MODE_SIZE = sizeof(rno_g_cmd_set_mode_t), 
  RNO_G_CMD_REPORT_SIZE = sizeof(rno_g_cmd_report_t), 
  RNO_G_CMD_LTE_STATS_SIZE = sizeof(rno_g_cmd_lte_stats_t), 
  RNO_G_CMD_LORA_STATS_SIZE = sizeof(rno_g_cmd_lora_stats_t), 
  RNO_G_CMD_LORA_TIMESYNC_SIZE = sizeof(rno_g_cmd_lora_timesync_t),  
  RNO_G_CMD_SET_GPS_SECS_OFFSET_SIZE = sizeof(rno_g_cmd_set_gps_secs_offset_t), 
  RNO_G_CMD_SET_BATTERY_THRESHOLDS_SIZE = sizeof(rno_g_cmd_battery_thresholds_t), 
  RNO_G_CMD_SBC_SIZE = sizeof(rno_g_cmd_sbc_t), 
  RNO_G_CMD_RESET_SIZE = sizeof(rno_g_cmd_reset_t)
}; 


#ifdef __cplusplus
}
#endif 

#endif

