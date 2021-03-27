#ifndef _RNO_G_H
#define _RNO_G_H

/** Structs describing state, commands etc. 
 *
 * this will be used both on and off the MCU, so it needs to only depend on standard stuff.
 *
 * Only targetting gcc on little-endian arm-none-eabi (newlib), and armv7l and x86x64 (Linux, glibc). 
 *
 */

#include <stdint.h> 

/** This is the main struct describing 
 * the current state of the MCU */ 


typedef enum sbc_boot_mode
{
  SBC_BOOT_INTERNAL = 0,
  SBC_BOOT_SDCARD = 1
}sbc_boot_mode_t; 


/** TODO:
 * Check if this is binary compatible on all the targets */ 

typedef struct rno_g_state
{
  uint8_t low_power_mode : 1;  // low power mode supercedes basically everything else. this also includes the vicor 5V
  uint8_t sbc_power : 1;
  sbc_boot_mode_t sbc_mode : 1; 
  uint8_t lte_power : 1; 
  uint8_t radiant_power : 1; 
  uint8_t lowthresh_power : 1; 
  uint8_t surf_amp_power : 6; 
  uint8_t dh_amp_power : 3; 
} rno_g_state_t; 


/** voltages, temperatures, etc. */ 
typedef struct rno_g_monitor
{
  uint32_t when; 
  int16_t temp_cC; 
  int16_t i_surf3v[6]; 
  int16_t i_down3v[3]; 
  int16_t i_sbc5v; 
  int16_t i_5v[2]; 
} rno_g_monitor_t;

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

} power_system_monitor_t;


typedef enum lte_state
{
  LTE_OFF, 
  LTE_TURNING_ON,
  LTE_ON,
  LTE_TURNING_OFF
} lte_state_t; 

typedef enum sbc_state
{
  SBC_OFF, 
  SBC_ON,
  SBC_TURNING_ON, 
  SBC_TURNING_OFF
} sbc_state_t; 




#endif

