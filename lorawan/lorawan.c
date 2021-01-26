
#include "config/lorawan_config.h" 

#if LORAWAN_PRINT_DEBUG
#include "shared/printf.h" 
#else
static inline printf(const char * str, ...) { ; } 
#endif


#include "lorawan.h" 
#include "rtc-board.h" 

#include <string.h>

#include "board.h" 
#include "sx1272-board.h" 
#include "gpio.h"
#include "utilities.h"

#include "LoRaMac.h"

/**** 
 *
 *  lorawan.c 
 *
 * This is based on the code from LoRaMac class A device implementation
 * by Semtech ( Miguel Luis and Gregory Cristian). 
 *
 * It was modified by Dan Smith <danielsmith@uchicago.edu> 
 * and Cosmin Deaconu <cozzyd@kicp.uchicago.edu> to fit our purposes */ 


 /**************************
  *
  *    Start off with the message queue handling stuff since that's easier
  */ 


// RX/TX buffer. This has both a maximum size and number of messages (due to bookkeeping).
// if each message were the same length, this would be simpler (could just do a circular buffer) 
// to add, copy data to back of tx_buffer, and put the offset in tx_offsets, then adjust n_tx_messages and tx_buffer used
// since we want a fifo, we have to read from the beginning. since we don't have a circular buffer we then need to copy 
// rest of the messages back to the beginning, which sucks. but at least it's simple... 
//
static uint8_t tx_buffer[LORAWAN_TX_BUFFER_SIZE]; 
static uint16_t tx_offsets[LORAWAN_MAX_TX_MESSAGES]; 
static uint8_t rx_buffer[LORAWAN_TX_BUFFER_SIZE]; 
static uint16_t rx_offsets[LORAWAN_MAX_TX_MESSAGES]; 

struct msg_buffer 
{
  uint8_t * buffer; 
  uint16_t * offsets;
  uint16_t used; 
  uint8_t n_messages; 
}; 

static struct msg_buffer tx = {.buffer = tx_buffer, .offsets = tx_offsets, .used = 0, .n_messages = 0};
static struct msg_buffer rx = {.buffer = rx_buffer, .offsets = rx_offsets, .used = 0, .n_messages = 0};

static inline  uint8_t * first_message(struct msg_buffer * b) {return b->buffer; }

static int first_message_length(struct msg_buffer * b)
{
  return b->n_messages == 0 ? -1 : 
         b->n_messages == 1 ? b->used : 
         b->offsets[1]; 
}

static void consume_first_message(struct msg_buffer *b)
{
  //the simplest case
  if (b->n_messages == 0) return; 

  //the simple case
  if (b->n_messages == 1) 
  {
    b->n_messages=0 ;
    b->used = 0; 
    return; 
  }

  int len = first_message_length(b); 
  b->used-=len; 
  memmove(b->buffer, b->buffer+len, b->used); 
  b->n_messages--; 
  memmove(b->offsets, b->offsets+1, b->n_messages*sizeof(*b->offsets)); 
}

/** DOES NOT CHECK CAPACITY!!!! */ 
static void append_message(struct msg_buffer * b, int len, const uint8_t * msg) 
{
  b->offsets[b->n_messages] = b->used; 
  memcpy(b->offsets+b->used,msg, len); 
  b->n_messages++; 
  b->used+= len; 
}

static int have_tx_capacity(int len)
{
  return len+tx.used < LORAWAN_TX_BUFFER_SIZE && tx.n_messages < LORAWAN_MAX_TX_MESSAGES;
}
static int have_rx_capacity(int len)
{
  return len+rx.used < LORAWAN_RX_BUFFER_SIZE && rx.n_messages < LORAWAN_MAX_RX_MESSAGES;
}

int lorawan_send_message(uint8_t len, const uint8_t * msg) 
{

  if (!have_tx_capacity(len)) 
  {
    return -1; //tx buffer full
  }

  append_message(&tx, len, msg); 
  return 0; 
}

uint8_t lorawan_next_received_message(uint8_t * msg, uint8_t maxlen)
{
 int len = first_message_length(&rx); 
 if (len <= 0)  return 0; // nothing to see here 
 uint8_t copy_len = maxlen < len ? maxlen : len; 
 memcpy(msg, first_message(&rx), copy_len); 
 consume_first_message(&rx); 
 return copy_len; 
}


/** Much of the following is based off the LoRaMac Class A app */

// Device states
static enum 
{
    DEVICE_STATE_RESTORE,
    DEVICE_STATE_START,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP,
}DeviceState;


static LoRaMacPrimitives_t macPrimitives;
static LoRaMacCallback_t macCallbacks;
static MibRequestConfirm_t mibReq;
static uint8_t devEui[] = LORAWAN_DEVICE_EUI;
static uint8_t joinEui[] = LORAWAN_JOIN_EUI;
static uint8_t AppKey[] = LORAWAN_APP_KEY;
static uint8_t GenAppKey[] = LORAWAN_APP_KEY;
static uint8_t NwkKey[] = LORAWAN_NWK_KEY;
// Timer to handle the application data transmission duty cycle
static TimerEvent_t TxNextPacketTimer;
static uint32_t TxDutyCycleTime;

#define APP_TX_DUTYCYCLE                            5000
//Defines a random delay for application data transmission duty cycle. 1s, value in [ms].
#define APP_TX_DUTYCYCLE_RND                        1000

#ifdef LORAWAN_PRINT_DEBUG
// MAC status strings
static const char* MacStatusStrings[] =
{
    "OK",                            // LORAMAC_STATUS_OK
    "Busy",                          // LORAMAC_STATUS_BUSY
    "Service unknown",               // LORAMAC_STATUS_SERVICE_UNKNOWN
    "Parameter invalid",             // LORAMAC_STATUS_PARAMETER_INVALID
    "Frequency invalid",             // LORAMAC_STATUS_FREQUENCY_INVALID
    "Datarate invalid",              // LORAMAC_STATUS_DATARATE_INVALID
    "Frequency or datarate invalid", // LORAMAC_STATUS_FREQ_AND_DR_INVALID
    "No network joined",             // LORAMAC_STATUS_NO_NETWORK_JOINED
    "Length error",                  // LORAMAC_STATUS_LENGTH_ERROR
    "Region not supported",          // LORAMAC_STATUS_REGION_NOT_SUPPORTED
    "Skipped APP data",              // LORAMAC_STATUS_SKIPPED_APP_DATA
    "Duty-cycle restricted",         // LORAMAC_STATUS_DUTYCYCLE_RESTRICTED
    "No channel found",              // LORAMAC_STATUS_NO_CHANNEL_FOUND
    "No free channel found",         // LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND
    "Busy beacon reserved time",     // LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME
    "Busy ping-slot window time",    // LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME
    "Busy uplink collision",         // LORAMAC_STATUS_BUSY_UPLINK_COLLISION
    "Crypto error",                  // LORAMAC_STATUS_CRYPTO_ERROR
    "FCnt handler error",            // LORAMAC_STATUS_FCNT_HANDLER_ERROR
    "MAC command error",             // LORAMAC_STATUS_MAC_COMMAD_ERROR
    "ClassB error",                  // LORAMAC_STATUS_CLASS_B_ERROR
    "Confirm queue error",           // LORAMAC_STATUS_CONFIRM_QUEUE_ERROR
    "Multicast group undefined",     // LORAMAC_STATUS_MC_GROUP_UNDEFINED
    "Unknown error",                 // LORAMAC_STATUS_ERROR
};


// MAC event info status strings.
static const char* EventInfoStatusStrings[] =
{ 
    "OK",                            // LORAMAC_EVENT_INFO_STATUS_OK
    "Error",                         // LORAMAC_EVENT_INFO_STATUS_ERROR
    "Tx timeout",                    // LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT
    "Rx 1 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT
    "Rx 2 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT
    "Rx1 error",                     // LORAMAC_EVENT_INFO_STATUS_RX1_ERROR
    "Rx2 error",                     // LORAMAC_EVENT_INFO_STATUS_RX2_ERROR
    "Join failed",                   // LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL
    "Downlink repeated",             // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED
    "Tx DR payload size error",      // LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR
    "Downlink too many frames loss", // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS
    "Address fail",                  // LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL
    "MIC fail",                      // LORAMAC_EVENT_INFO_STATUS_MIC_FAIL
    "Multicast fail",                // LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL
    "Beacon locked",                 // LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED
    "Beacon lost",                   // LORAMAC_EVENT_INFO_STATUS_BEACON_LOST
    "Beacon not found"               // LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND
};

/*!
 * Prints the provided buffer in HEX
 * 
 * \param buffer Buffer to be printed
 * \param size   Buffer size to be printed
 */
void PrintHexBuffer( uint8_t *buffer, uint8_t size )
{
    uint8_t newline = 0;

    for( uint8_t i = 0; i < size; i++ ) { 
      if( newline != 0 ) { 
        printf( "\r\n" );
        newline = 0;
      }
      
      printf( "%02X ", buffer[i] );

      if( ( ( i + 1 ) % 16 ) == 0 ) {
        newline = 1;
      }
    }
    printf( "\r\n" );
}

#endif



static void JoinNetwork( void )
{
  LoRaMacStatus_t status;
  MlmeReq_t mlmeReq;
  mlmeReq.Type = MLME_JOIN;
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;
  
  // Starts the join procedure
  status = LoRaMacMlmeRequest( &mlmeReq );
  printf( "\r\n###### ===== MLME-Request - MLME_JOIN ==== ######\r\n" );
#ifdef LORAWAN_PRINT_DEBUG
  printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif

  if( status == LORAMAC_STATUS_OK ) {
      printf( "###### ===== JOINING ==== ######\r\n" );
      DeviceState = DEVICE_STATE_SLEEP;
  } else 
  {  
    DeviceState = DEVICE_STATE_CYCLE;
  }
}

// Indicates if the node is sending confirmed or unconfirmed messages
static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;


/* Send the next thing in our buffer, I guess ? */ 
static bool SendFrame( void ) { 

  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  uint8_t buffer_length = first_message_length(&tx); 


  //nothing to send, so don't try!
  if (!buffer_length) 
    return 0; 

  LoRaMacStatus_t cur_status = LoRaMacQueryTxPossible( buffer_length, &txInfo);
  if( cur_status != LORAMAC_STATUS_OK ) {
      printf("Sending empty Data, Loramac didn't return status ok. ");
#ifdef LORAWAN_PRINT_DEBUG
      printf( "STATUS      : %s\r\n", MacStatusStrings[cur_status] );
#endif 
      
      // Send empty frame in order to flush MAC commands
      mcpsReq.Type = MCPS_UNCONFIRMED;
      mcpsReq.Req.Unconfirmed.fBuffer = NULL;
      mcpsReq.Req.Unconfirmed.fBufferSize = 0;
      mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    } else { 

      uint8_t * buffer = first_message(&tx); 

      if( IsTxConfirmed == false ) {
        printf("Sending unconfirmed Data.");
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fPort = LORAWAN_APP_PORT;
        mcpsReq.Req.Unconfirmed.fBuffer = buffer;
        mcpsReq.Req.Unconfirmed.fBufferSize = buffer_length;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      } else {
        printf("Sending Confirmed Data.");
        mcpsReq.Type = MCPS_CONFIRMED;
        mcpsReq.Req.Confirmed.fPort = LORAWAN_APP_PORT;
        mcpsReq.Req.Confirmed.fBuffer = buffer;
        mcpsReq.Req.Confirmed.fBufferSize = buffer_length;
        mcpsReq.Req.Confirmed.NbTrials = 8;
        mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      }
    }

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );
    printf( "\r\n###### ===== MCPS-Request ==== ######\r\n" );
#ifdef LORAWAN_PRINT_DEBUG
    printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif 

    if( status == LORAMAC_STATUS_OK )
    {
        //can we pop off the buffer? 
        // I guess we'll find out! 
        consume_first_message(&rx); 
        return 0;
    }
    return 1;
}

static bool NextTx = 1;

/*!
 * Indicates if LoRaMacProcess call is pending.
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static uint8_t IsMacProcessPending = 0;


/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent( void* context )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop( &TxNextPacketTimer );

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK ) { 
      if( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE ) { 
              // Network not joined yet. Try to join again
             JoinNetwork( );
      } else { 
        DeviceState = DEVICE_STATE_SEND;
        NextTx = true;
      }
    }
}




void lorawan_process()
{

    //check the interrupts

        
    TimerProcess( );
    // Process Radio IRQ
    if( Radio.IrqProcess != NULL ) {
      Radio.IrqProcess( );
    }
      
    // Processes the LoRaMac events      
    LoRaMacProcess( );
  
    switch( DeviceState ) 
    { 

      case DEVICE_STATE_RESTORE: {        

   #if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
                      mibReq.Type = MIB_GEN_APP_KEY;
                      mibReq.Param.GenAppKey = GenAppKey;
                      LoRaMacMibSetRequestConfirm( &mibReq );
#else
                      mibReq.Type = MIB_APP_KEY;
                      mibReq.Param.AppKey = AppKey;
                      LoRaMacMibSetRequestConfirm( &mibReq );
#endif

   
          mibReq.Type = MIB_NWK_KEY;
          mibReq.Param.NwkKey = NwkKey;
          LoRaMacMibSetRequestConfirm( &mibReq );
            
          // Initialize LoRaMac device unique ID if not already defined in Commissioning.h
          if( ( devEui[0] == 0 ) && ( devEui[1] == 0 ) &&
              ( devEui[2] == 0 ) && ( devEui[3] == 0 ) &&
              ( devEui[4] == 0 ) && ( devEui[5] == 0 ) &&
              ( devEui[6] == 0 ) && ( devEui[7] == 0 ) ) {
            BoardGetUniqueId( devEui );
          }

          mibReq.Type = MIB_DEV_EUI;
          mibReq.Param.DevEui = devEui;
          LoRaMacMibSetRequestConfirm( &mibReq );
            
          mibReq.Type = MIB_JOIN_EUI;
          mibReq.Param.JoinEui = joinEui;
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_CHANNELS_TX_POWER; 
          mibReq.Param.ChannelsTxPower = 14; 
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_CHANNELS_DEFAULT_TX_POWER; 
          mibReq.Param.ChannelsDefaultTxPower = 14; 
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR; 
          mibReq.Param.SystemMaxRxError = 100; 
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_MIN_RX_SYMBOLS; 
          mibReq.Param.MinRxSymbols = 50; 
          LoRaMacMibSetRequestConfirm( &mibReq );



          /*
          mibReq.Type = MIB_CHANNELS_MASK;
          mibReq.Param.ChannelsMask[0] = 0x00FF;
          mibReq.Param.ChannelsMask[1] = 0x0000;
          mibReq.Param.ChannelsMask[2] = 0x0000;
          mibReq.Param.ChannelsMask[3] = 0x0000;
          mibReq.Param.ChannelsMask[4] = 0x0001;
          mibReq.Param.ChannelsMask[5] = 0x0000;

          // Prints out the mask as requested
          mibReq.Type  = MIB_CHANNELS_MASK;
          if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK ) {
            printf("CHANNEL MASK: ");
              
            for( uint8_t i = 0; i < 5; i++) {
              printf("%04X ", mibReq.Param.ChannelsMask[i] );
            }
            printf("\r\n");
          }
          */ 
        
        DeviceState = DEVICE_STATE_START;
        break;
      }
          
      case DEVICE_STATE_START: {
        TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

        mibReq.Type = MIB_PUBLIC_NETWORK;
        mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_ADR;
        mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
        mibReq.Param.SystemMaxRxError = 20;
        LoRaMacMibSetRequestConfirm( &mibReq );

        LoRaMacStart( );

        mibReq.Type = MIB_NETWORK_ACTIVATION;
        LoRaMacStatus_t status = LoRaMacMibGetRequestConfirm( &mibReq );

        if( status == LORAMAC_STATUS_OK ) {
          if(mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE ) {
            DeviceState = DEVICE_STATE_JOIN;
          } else {
            DeviceState = DEVICE_STATE_SEND;
            NextTx = true;
          }
        }
        break;
      }

      case DEVICE_STATE_JOIN: {    
        mibReq.Type = MIB_DEV_EUI;
        LoRaMacMibGetRequestConfirm( &mibReq );
        printf( "DevEui      : %02X", mibReq.Param.DevEui[0] );
        for( int i = 1; i < 8; i++ )
        {
          printf( "-%02X", mibReq.Param.DevEui[i] );
        }
        printf( "\r\n" );
        mibReq.Type = MIB_JOIN_EUI;
        LoRaMacMibGetRequestConfirm( &mibReq );
        printf( "AppEui      : %02X", mibReq.Param.JoinEui[0] );
        for( int i = 1; i < 8; i++ )
        {
          printf( "-%02X", mibReq.Param.JoinEui[i] );
        }
        printf( "\r\n" );
        printf( "AppKey      : %02X", NwkKey[0] );
        for( int i = 1; i < 16; i++ )
        {
          printf( " %02X", NwkKey[i] );
        }
        printf( "\n\r\n" );

        JoinNetwork( );    
        break;
      }

      case DEVICE_STATE_SEND: {
        if(NextTx == true) {
          NextTx = SendFrame( );
        }
        DeviceState = DEVICE_STATE_CYCLE;
        break;
      }

      case DEVICE_STATE_CYCLE: {
        DeviceState = DEVICE_STATE_SLEEP;
        TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );

        // Schedule next packet transmission
        TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
        TimerStart( &TxNextPacketTimer );
        break;
      }

      case DEVICE_STATE_SLEEP: {
      
        CRITICAL_SECTION_BEGIN( ); // No clue what this means. 
        if( IsMacProcessPending == 1 ) {
          IsMacProcessPending = 0; // Clear flag and prevent MCU to go into low power modes.
        } else {      
          //BoardLowPowerHandler( ); // The MCU wakes up through events
        }
        CRITICAL_SECTION_END( );
        break;
      }

      default: {
        DeviceState = DEVICE_STATE_START;
        break;
      }
      
    } // end of switch

  return;

}

//// CALLBACKS 

static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    printf( "\r\n###### ===== MCPS-Confirm ==== ######\r\n" );
#ifdef LORAWAN_PRINT_DEBUG
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mcpsConfirm->Status] );
#endif
    if( mcpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {

    } else {
      switch( mcpsConfirm->McpsRequest ) {
      case MCPS_UNCONFIRMED: {
        // Check Datarate
        // Check TxPower
        break;
      }
      case MCPS_CONFIRMED: {

        // Check Datarate
        // Check TxPower
        // Check AckReceived
        // Check NbTrials
        break;
      }
      case MCPS_PROPRIETARY: {
        break;
      }
      default:
        break;
      }

    }

    MibRequestConfirm_t mibGet;
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );

    printf( "\r\n###### ===== UPLINK FRAME %lu ==== ######\r\n", mcpsConfirm->UpLinkCounter );
    printf( "\r\n" );

    printf( "CLASS       : %c\r\n", "ABC"[mibReq.Param.Class] );
    printf( "\r\n" );
    printf( "TX PORT     : %d\r\n", LORAWAN_APP_PORT );

    /*
    if( AppData.BufferSize != 0 ) {
      printf( "TX DATA     : " );
      if( AppData.MsgType == LORAMAC_HANDLER_CONFIRMED_MSG ) {
        printf( "CONFIRMED - %s\r\n", ( mcpsConfirm->AckReceived != 0 ) ? "ACK" : "NACK" );
      } else {
        printf( "UNCONFIRMED\r\n" );
      }
      PrintHexBuffer( AppData.Buffer, AppData.BufferSize );
    }
    */

    printf( "\r\n" );
    printf( "DATA RATE   : DR_%d\r\n", mcpsConfirm->Datarate );

    mibGet.Type  = MIB_CHANNELS;
    if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK ) {
      printf( "U/L FREQ    : %lu\r\n", mibGet.Param.ChannelList[mcpsConfirm->Channel].Frequency );
    }

    printf( "TX POWER    : %d\r\n", mcpsConfirm->TxPower );

    mibGet.Type  = MIB_CHANNELS_MASK;
    if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK ) {
      printf("CHANNEL MASK: ");

      for( uint8_t i = 0; i < 5; i++) {
        printf("%04X ", mibGet.Param.ChannelsMask[i] );
      }
      printf("\r\n");
    }

    printf( "\r\n" );
}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
#ifdef LORAWAN_PRINT_DEBUG
    printf( "\r\n###### ===== MCPS-Indication ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mcpsIndication->Status] );
#endif
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ) { 
      return;
    }

    switch( mcpsIndication->McpsIndication ) {
    case MCPS_UNCONFIRMED: { 
      break;
    }
    case MCPS_CONFIRMED: { 
      break;
    }
    case MCPS_PROPRIETARY: {
      break;
    }
    case MCPS_MULTICAST: { 
      break;
    }
    default:
      break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending
    if( mcpsIndication->FramePending == true ) { 
        // The server signals that it has pending data to be sent.
        // We schedule an uplink as soon as possible to flush the server.
        OnTxNextPacketTimerEvent( NULL );
    }
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot

    if( mcpsIndication->RxData == true ) {
      if( mcpsIndication->BufferSize == 1 ) {
//        AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
      }
    }

    const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    printf( "\r\n###### ===== DOWNLINK FRAME %lu ==== ######\r\n", mcpsIndication->DownLinkCounter );

    printf( "RX WINDOW   : %s\r\n", slotStrings[mcpsIndication->RxSlot] );
    
    printf( "RX PORT     : %d\r\n", mcpsIndication->Port );

    if( mcpsIndication->BufferSize != 0 )
    {
#ifdef LORAWAN_PRINT_DEBUG
        printf( "RX DATA     : \r\n" );
        PrintHexBuffer( mcpsIndication->Buffer, mcpsIndication->BufferSize );
#endif
    }

    printf( "\r\n" );
    printf( "DATA RATE   : DR_%d\r\n", mcpsIndication->RxDatarate );
    printf( "RX RSSI     : %d\r\n", mcpsIndication->Rssi );
    printf( "RX SNR      : %d\r\n", mcpsIndication->Snr );

    printf( "\r\n" );
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
#ifdef LORAWAN_PRINT_DEBUG
    printf( "\r\n###### ===== MLME-Confirm ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mlmeConfirm->Status] );
#endif
    if( mlmeConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
    }
    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                MibRequestConfirm_t mibGet;
                printf( "###### ===== JOINED ==== ######\r\n" );
                printf( "\r\nOTAA\r\n\r\n" );

                mibGet.Type = MIB_DEV_ADDR;
                LoRaMacMibGetRequestConfirm( &mibGet );
                printf( "DevAddr     : %08lX\r\n", mibGet.Param.DevAddr );

                printf( "\n\r\n" );
                mibGet.Type = MIB_CHANNELS_DATARATE;
                LoRaMacMibGetRequestConfirm( &mibGet );
                printf( "DATA RATE   : DR_%d\r\n", mibGet.Param.ChannelsDatarate );
                printf( "\r\n" );
                
                mibGet.Type  = MIB_CHANNELS_MASK;
                if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK ) {
                  printf("CHANNEL MASK: ");
            
                  for( uint8_t i = 0; i < 5; i++) {
                    printf("%04X ", mibGet.Param.ChannelsMask[i] );
                  }
                  printf("\r\n");
                }
          
                // Status is OK, node has joined the network
                DeviceState = DEVICE_STATE_SEND;
            }
            else
            {
                // Join was not successful. Try to join again
                JoinNetwork( );
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
            }
            break;
        }
        default:
            break;
    }
}

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure.
 */
static void MlmeIndication( MlmeIndication_t *mlmeIndication ) { 
  if( mlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED ) { 
#ifdef LORAWAN_PRINT_DEBUG
    printf( "\r\n###### ===== MLME-Indication ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mlmeIndication->Status] );
#endif
  }
  if( mlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {}
  
  switch( mlmeIndication->MlmeIndication ) { 
  case MLME_SCHEDULE_UPLINK: { 
    // The MAC signals that we shall provide an uplink as soon as possible
    OnTxNextPacketTimerEvent( NULL );
    break;
  }
  default:
    break;
  }
}
void OnMacProcessNotify( void ) { 
  IsMacProcessPending = 1;
}

static volatile int battery_level_calls; 
uint8_t GetBatteryLevel (void) 
{
  battery_level_calls++; 
  return 100; 

}



int lorawan_init() 
{

  //Do the equivalent of BoardInit

  SX1272IoInit( );
  RtcInit(); 


  // Set up callbacks 

  macPrimitives.MacMcpsConfirm = McpsConfirm;
  macPrimitives.MacMcpsIndication = McpsIndication;
  macPrimitives.MacMlmeConfirm = MlmeConfirm;
  macPrimitives.MacMlmeIndication = MlmeIndication;
  macCallbacks.GetBatteryLevel = GetBatteryLevel;
  macCallbacks.GetTemperatureLevel = NULL;
  macCallbacks.NvmContextChange = NULL;
  macCallbacks.MacProcessNotify = OnMacProcessNotify;


  LoRaMacStatus_t status;
  status = LoRaMacInitialization( &macPrimitives, &macCallbacks, ACTIVE_REGION );
  if( status != LORAMAC_STATUS_OK ) {
#ifdef LORAWAN_PRINT_DEBUG
   printf( "LoRaMac wasn't properly initialized, error: %s", MacStatusStrings[status] );
#endif
   return status; 
  }

  DeviceState = DEVICE_STATE_RESTORE;

  return 0; 
}






