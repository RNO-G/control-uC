#ifndef _RNO_G_LORAWAN_CONFIG_H
#define _RNO_G_LORAWAN_CONFIG_H


/* Defines for lorawan here */ 


#define LORAWAN_BUFFER_SIZE 512
#define LORAWAN_MAX_MESSAGES 4

#define LORAWAN_PRINT_DEBUG 0

#define LORAWAN_CONFIRMED_MSG_ON 0 

#define ACTIVE_REGION LORAMAC_REGION_US915
#define ABP_ACTIVATION_LRWAN_VERSION_V10x   0x01000300 // 1.0.3.0
#define ABP_ACTIVATION_LRWAN_VERSION   ABP_ACTIVATION_LRWAN_VERSION_V10x

// the combination on my luggage 
#define LORAWAN_JOIN_EUI     { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }

// the non-oui part is auto-generated per board if all zeros
#define LORAWAN_DEVICE_EUI  { IEEE_OUI, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define LORAWAN_APP_KEY   { 0xc0, 0xfe, 0xfe, 0xc0, 0xff, 0xee, 0x15, 0xab, 0xad, 0xf0, 0x0d, 0xd0, 0x0d, 0xb1, 0xec, 0xcc }
#define LORAWAN_NWK_KEY LORAWAN_APP_KEY

#define OVER_THE_AIR_ACTIVATION 1

//Greenland is 
#define IEEE_OUI 0xc0, 0x1d, 0xaf

#define LORAWAN_PUBLIC_NETWORK true 


#define LORAWAN_DEFAULT_DATARATE                    DR_2
#define LORAWAN_ADR_ON                              0

//TODO make this configurable and automatically increasing!
#define LORAWAN_RX_ERROR  350 

#define LORAWAN_MAX_FAILED_LINK_CHECKS 3

#define LORAWAN_LINK_CHECK_INTERVAL 600 

#define LORAWAN_TX_POWER 14

#define LORAWAN_CHANNEL_MASK {0xff00,0x0,0x0,0x0,0x1}

#endif
