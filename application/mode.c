#include "application/mode.h"
#include "application/lte.h" 
#include "application/sbc.h" 
#include "application/lowpower.h" 
#include "shared/spi_flash.h" 
#include "hal_delay.h" 

static rno_g_mode_t the_mode = RNO_G_INIT; 

void mode_init() 
{

  if (low_power_mode) 
  {
    the_mode = RNO_G_LOW_POWER_MODE; 
  }
  else
  {
    sbc_state_t sbc = sbc_get_state(); 

    if (sbc == SBC_OFF || sbc ==SBC_TURNING_OFF) 
    {
      the_mode= RNO_G_SBC_OFF_MODE; 
    }
    else
    {
      lte_state_t lte = lte_get_state() ; 
      //busy wait here, I guess? 
      while (lte==LTE_INIT) 
      {
        delay_ms(1); 
        lte =lte_get_state(); 
      }

      if (lte == LTE_ON || lte == LTE_TURNING_ON)
      {
        the_mode = RNO_G_NORMAL_MODE; 
      }
      else
      {
        the_mode = RNO_G_SBC_ONLY_MODE; 
      }
    }
  }
}


rno_g_mode_t mode_query()  { return the_mode; } 

int mode_set(rno_g_mode_t mode) 
{
  if (the_mode == RNO_G_INIT) mode_init(); 
  if (mode == RNO_G_INIT) 
  {
    return 0; //don't do anything in this case... wasn't initialized properly 
  }

  if (the_mode == mode) return 0; 

  // Enter low power mode
  if (mode == RNO_G_LOW_POWER_MODE) 
  {
    lte_turn_off(0); 
    sbc_turn_off(); 
    low_power_mode_enter(); 
  }
  else
  {
    if (the_mode == RNO_G_LOW_POWER_MODE) 
    {
      low_power_mode_exit(); 
      if (mode != RNO_G_SBC_OFF_MODE) sbc_turn_on(config_block()->app_cfg.sbc_boot_mode); 
      if (mode == RNO_G_NORMAL_MODE) lte_turn_on(2); //this will only turn on if it's not on, and override the mode check 
    }

    else if (the_mode == RNO_G_SBC_OFF_MODE) 
    {
      sbc_turn_on(config_block()->app_cfg.sbc_boot_mode);  //since we've already handled low power mode all other modes want the SBC On 
      if (mode == RNO_G_NORMAL_MODE) lte_turn_on(2); 
    }

    else if (mode == RNO_G_SBC_OFF_MODE) 
    {
      sbc_turn_off(); 
      lte_turn_off(0); // might already be off but that's ok 
    }
    else if (mode == RNO_G_SBC_ONLY_MODE) 
    {
      lte_turn_off(0); //might already be off but that's ok
    }
    else if (mode == RNO_G_NORMAL_MODE) 
    {
      lte_turn_on(2); 
    }
  }


  config_block()->app_cfg.wanted_state = mode; 
  config_block_sync(); 
  the_mode = mode; 
  return 0; 
}



