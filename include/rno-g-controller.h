#ifndef rno_g_controller_h
#define rno_g_controller_h


/** This is the API for the librno-g-controller library. 
 *
 * This communicates with the controller board over the UART link OR via a dbus interface. 
 *
 * Many options are available as a simple register driven interface. 
 *
 */ 


//Structs from the MCU 
#include "rno-g-control.h" 

typedef struct rno_g_controller rno_g_controller_t; 


/** Open a RAW connection to the device 
 *  If uart_device is NULL, a simulator will be created. 
 * */ 
rno_g_controller_t *  rno_g_controller_open_raw(const char * uart_device);

/** This really only opens a dbus handle  (rno-g-controllerd must be running) */ 
rno_g_controller_t *  rno_g_controller_open_dbus(int userbus);

void rno_g_controller_close(rno_g_controller_t * dev); 



typedef enum 
{
  RNO_G_VAL_LTE_ON,   // 0 = off, 1 = on
  RNO_G_VAL_SBC_BOOT_MODE,  // 0 = MMC, 1 = SD 
  RNO_G_VAL_MCU_BOOT_SLOT,  // 0 = internal, 1-4 from spiflash
  RNO_G_VAL_STATION_NUMBER,  // 0 = internal, 1-4 from spiflash
  RNO_G_VAL_MODE,  // 0 = internal, 1-4 from spiflash
  RNO_G_VAL_SURFACE_AMPS,  // 6bit mask
  RNO_G_VAL_DH_AMPS,  // 3bit mask
  RNO_G_VAL_RADIANT_ON,  // 0 or 1
  RNO_G_VAL_LT_ON,  // 0 or 1
  RNO_G_VAL_NOT_A_VAL
} rno_g_controller_key_t; 



int rno_g_set_val(rno_g_controller_t *dev, rno_g_controller_key_t what, int val); 

int rno_g_get_val(rno_g_controller_t *dev, rno_g_controller_key_t what, int * val); 

/** Schedule a monitor (<0 to not schedule and just read what is currently there, 0 to use default) */ 
int rno_g_monitor(rno_g_controller_t *dev, int navg, rno_g_report_t * result); 

/** Same as above, but allocate a string and store the pointer to *str_p. User responsible for freeing. */ 
int rno_g_monitor_json(rno_g_controller_t *dev, int navg, char ** str_p); 

/** Reset the microcontroller */ 
int rno_g_reset(rno_g_controller_t *dev, int to); 

int rno_g_program_flash(rno_g_controller_t *dev, int slot, int len, const uint8_t * buf); 
int rno_g_read_flash(rno_g_controller_t *dev, int slot, int maxlen, uint8_t * buf); 


#define RNO_G_DBUS_SERVICE "org.rno-g.controller" 
#define RNO_G_DBUS_OBJECT_PATH "/org/rno-g/controller" 
#define RNO_G_DBUS_INTERFACE_NAME RNO_G_DBUS_SERVICE


#endif 
