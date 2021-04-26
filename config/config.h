#ifndef _rno_g_uc_config_h
#define _rno_g_uc_config_h

#define USE_SYNCHRONOUS_I2C 1 

// the loop delay time, when not in low power mode
#define DELAY_MS 1 


#define APP_VERSION "Chicago.042621.1" 


//temporary 
#define LTE_TURNON_NTICKS 10000

//seconds
#define LOW_POWER_SLEEP_AMOUNT 60 

// about 10 seconds
#define LOW_POWER_AWAKE_TICKS  8192   

#define ENABLE_WATCHDOG 0 

#endif
