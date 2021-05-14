#ifndef _rno_g_uc_config_h
#define _rno_g_uc_config_h

#define USE_SYNCHRONOUS_I2C 1 

// the loop delay time, when not in low power mode
#define DELAY_MS 1 

//fast time checks based on loop ticks (don't work in low power mode, obviously) 

#define ABOUT_A_SECOND 1023
#define ABOUT_10_SECONDS 8191
#define ABOUT_20_SECONDS 16383
#define ABOUT_30_SECONDS 32767
#define ABOUT_A_MINUTE 65535


#define APP_VERSION "Chicago.042921.1" 

#define MODE_CHANGE_MINTICKS 100

//seconds
#define LOW_POWER_SLEEP_AMOUNT 10 

// about 1 second? 
#define LOW_POWER_AWAKE_TICKS  200   

#define ENABLE_WATCHDOG 1 

#endif
