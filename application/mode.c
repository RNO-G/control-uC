#include "application/mode.h"
#include "application/lte.h" 
#include "application/sbc.h" 
#include "application/gpio_expander.h" 
#include "application/lowpower.h" 
#include "shared/spi_flash.h" 
#include "hal_delay.h" 

static rno_g_mode_t the_mode = RNO_G_INIT; 

void mode_init() 
{

  if ( config_block()->app_cfg.wanted_state == RNO_G_LOW_POWER_MODE) 
  {
    the_mode = RNO_G_LOW_POWER_MODE; 
    low_power_mode_enter(); 
    sbc_get_state(); 
    low_power_mon_off(); 
  }
  else
  {
    low_power_mode_exit(); 
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


static void turn_off_peripherals() 
{
  i2c_gpio_expander_t off = {0}; 
  i2c_gpio_expander_t mask = {.surface_amps = 0x3f, .dh_amps =0x7, .radiant=0, .lt=1}; 
  //everything but the radiant
  set_gpio_expander_state(off,mask); 
  delay_ms(5); 
  i2c_gpio_expander_t mask_radiant = {.radiant=1}; 
  //then the radiant
  set_gpio_expander_state(off,mask_radiant); 
}


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
    sbc_turn_off(); 
    lte_turn_off(0); 
    turn_off_peripherals(); 
    low_power_mode_enter(); 
  }
  else
  {
    if (the_mode == RNO_G_LOW_POWER_MODE) 
    {
      low_power_mode_exit(); 
      delay_ms(10);  // make sure the vicor is on so we can turn on the SBC!
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
      turn_off_peripherals(); 
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



