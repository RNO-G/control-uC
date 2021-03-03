#ifndef _RNO_G_LORAWAN_CONFIG_H
#define _RNO_G_LORAWAN_CONFIG_H


/* Defines for lorawan here */ 


#define LORAWAN_TX_BUFFER_SIZE 256
#define LORAWAN_MAX_TX_MESSAGES 4

#define LORAWAN_RX_BUFFER_SIZE 512
#define LORAWAN_MAX_RX_MESSAGES 8

#define LORAWAN_PRINT_DEBUG 0

#define LORAWAN_CONFIRMED_MSG_ON 0 

#define ACTIVE_REGION LORAMAC_REGION_US915
#define ABP_ACTIVATION_LRWAN_VERSION_V10x   0x01000300 // 1.0.3.0
#define ABP_ACTIVATION_LRWAN_VERSION   ABP_ACTIVATION_LRWAN_VERSION_V10x

// the combination on my luggage 
#define LORAWAN_JOIN_EUI     { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }

// the non-oui part is auto-generated per board
#define LORAWAN_DEVICE_EUI  { IEEE_OUI, 0x00, 0x00, 0x00, 0x00, 0x01 }

#define LORAWAN_APP_KEY   { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }
#define LORAWAN_NWK_KEY LORAWAN_APP_KEY

#define OVER_THE_AIR_ACTIVATION 1

//Greenland is 
#define IEEE_OUI 0xc0, 0x1d, 0xaf

#define LORAWAN_PUBLIC_NETWORK true 


#define LORAWAN_DEFAULT_DATARATE                    DR_3
#define LORAWAN_ADR_ON                              0

#define LORAWAN_RX_ERROR  100 


#define LORAWAN_LINK_CHECK_INTERVAL 3600 

#endif
