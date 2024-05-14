#ifndef _rno_g_uc_config_h
#define _rno_g_uc_config_h

#define USE_SYNCHRONOUS_I2C 1 

//uncomment if you don't want to autoprobe GPIO expanders 
#define I2C_EXPANDER_AUTOPROBE

// the loop delay time, when not in low power mode
#define DELAY_MS 1 

//fast time checks based on loop ticks (don't work in low power mode, obviously) 

#define ABOUT_A_SECOND 1023
#define ABOUT_10_SECONDS 8191
#define ABOUT_20_SECONDS 16383
#define ABOUT_30_SECONDS 32767
#define ABOUT_A_MINUTE 65535


#if defined(_RNO_G_REV_D)
#define APP_REV 'D' 
#define APP_REV_STR "D" 
#elif defined(_RNO_G_REV_E)
#define APP_REV 'E'
#define APP_REV_STR "E"
#define REV_AT_LEAST_E
#elif defined(_RNO_G_REV_F)
#define APP_REV 'F'
#define APP_REV_STR "F"
#define REV_AT_LEAST_E
#define REV_AT_LEAST_F
#else
#error "Unknown revision!"
#endif

#define REV_AT_LEAST(X) (X >= APP_REV)
#define REV_BELOW(X) (X < APP_REV)

#define APP_VERSION "Chicago.051424.0-REV" APP_REV_STR

#define MODE_CHANGE_MINTICKS 100

//seconds. Must be less than 7 with EW. 
#define LOW_POWER_SLEEP_AMOUNT 7 

#define LOW_POWER_AWAKE_TICKS  300   

#define ENABLE_WATCHDOG 1 
#define ENABLE_WATCHDOG_EW 1 


#endif
