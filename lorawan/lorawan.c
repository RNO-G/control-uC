
#include "config/lorawan_config.h" 

#if LORAWAN_PRINT_DEBUG
#include "shared/printf.h" 
#else
//hope this gets inlined 
static inline int printf(const char * str, ...) { (void) str; return 0; } 
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
// 
//
static uint8_t tx_buffer[LORAWAN_TX_BUFFER_SIZE]; 
static uint16_t tx_offsets[LORAWAN_MAX_TX_MESSAGES]; 
static uint8_t tx_flags[LORAWAN_MAX_TX_MESSAGES]; 
static uint8_t tx_ports[LORAWAN_MAX_TX_MESSAGES]; 

static uint8_t rx_buffer[LORAWAN_TX_BUFFER_SIZE]; 
static uint16_t rx_offsets[LORAWAN_MAX_TX_MESSAGES]; 
static uint8_t rx_flags[LORAWAN_MAX_TX_MESSAGES]; 
static uint8_t rx_ports[LORAWAN_MAX_TX_MESSAGES]; 

static volatile int joined = 0; 

struct msg_buffer 
{
  uint8_t * buffer; 
  uint16_t * offsets;
  uint16_t used; 
  uint8_t n_messages; 
  uint8_t got_mem; 
  uint8_t * flags; 
  uint8_t * ports; 
  uint32_t dropped; 
  uint32_t count; 
}; 

static struct msg_buffer tx = {.buffer = tx_buffer, .offsets = tx_offsets, .used = 0, .n_messages = 0, .flags = tx_flags, .ports = tx_ports, .dropped = 0, .count=0};
static struct msg_buffer rx = {.buffer = rx_buffer, .offsets = rx_offsets, .used = 0, .n_messages = 0, .flags = rx_flags, .ports = rx_ports, .dropped = 0, .count=0};

static inline  uint8_t * first_message(struct msg_buffer * b) {return b->buffer; }

static int first_message_length(struct msg_buffer * b)
{
  return b->n_messages == 0 ? -1 : 
         b->n_messages == 1 ? b->used : 
         b->offsets[1]; 
}

static uint8_t first_message_flags(struct msg_buffer * b) 
{
  return b->flags[0]; 
}

static uint8_t first_message_port(struct msg_buffer * b) 
{
  return b->ports[0]; 
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

  if (b->n_messages) 
  {
    //NOTE: these 3 can be circular buffers... change later. 
    memmove(b->flags, b->flags+1, b->n_messages); 
    memmove(b->ports, b->ports+1, b->n_messages); 
    memmove(b->offsets, b->offsets+1, b->n_messages*sizeof(*b->offsets)); 
  }
}



static int have_tx_capacity(int len)
{
  return len+tx.used < LORAWAN_TX_BUFFER_SIZE && tx.n_messages < LORAWAN_MAX_TX_MESSAGES;
}
static int have_rx_capacity(int len)
{
  return len+rx.used < LORAWAN_RX_BUFFER_SIZE && rx.n_messages < LORAWAN_MAX_RX_MESSAGES;
}





int msg_getmem(struct msg_buffer *b, uint8_t len, uint8_t port, uint8_t **msg, uint8_t flags) 
{
  if (b->got_mem) 
  {
    tx.dropped++; 
    return -2; 
  }

  b->offsets[b->n_messages] = b->used; 
  b->flags[b->n_messages] = flags; 
  b->ports[b->n_messages] = port; 
  *msg = b->buffer+b->used; 
  b->used+= len; 
  b->got_mem = 1; 
  return 0; 
}


int msg_push(struct msg_buffer *b) 
{
  if (!b->got_mem) 
  {
    return -1;
  }
  b->n_messages++; 
  b->count++; 
  b->got_mem = 0; 
  return 0; 

}

int lorawan_tx_getmem(uint8_t len, uint8_t port, uint8_t **msg, uint8_t flags) 
{
  if (!have_tx_capacity(len))
  {
    tx.dropped++; 
    return -1; 
  }
  return msg_getmem(&tx,len,port,msg,flags);
}

int lorawan_tx_push() 
{
  return msg_push(&tx); 
}

int lorawan_tx_copy(uint8_t len, uint8_t port, const uint8_t* msg, uint8_t flags) 
{
  uint8_t * dest = 0; 
  int ret = lorawan_tx_getmem(len,port,&dest,flags); 

  if (ret)
  {
    return ret;
  }

  memcpy(dest,msg,len); 
  return lorawan_tx_push(); 
}



int lorawan_rx_peek(uint8_t *len, uint8_t * port, uint8_t **msg, uint8_t * flags) 
{
  if (!rx.n_messages) return 0; 
  if (len) *len = first_message_length(&rx); 
  if (port) *port = first_message_port(&rx); 
  if (flags) *flags = first_message_flags(&rx); 
  if (msg) *msg= first_message(&rx); 
  return rx.n_messages; 
}

int lorawan_rx_pop() 
{
  if (!rx.n_messages) return -1; 
  consume_first_message(&rx); 
  return 0; 
}


uint8_t lorawan_copy_next_received_message(uint8_t * msg, uint8_t maxlen, uint8_t * port, uint8_t  * flags)
{
 int len = first_message_length(&rx); 
 if (len <= 0)  return 0; // nothing to see here 
 if (flags) *flags = first_message_flags(&rx); 
 if (port) *port = first_message_port(&rx); 
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
#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
static uint8_t GenAppKey[] = LORAWAN_APP_KEY;
#else
static uint8_t AppKey[] = LORAWAN_APP_KEY;
#endif
static uint8_t NwkKey[] = LORAWAN_NWK_KEY;
// Timer to handle the application data transmission duty cycle
static TimerEvent_t TxNextPacketTimer;
static uint32_t TxDutyCycleTime;

#define APP_TX_DUTYCYCLE                            5000
//Defines a random delay for application data transmission duty cycle. 1s, value in [ms].
#define APP_TX_DUTYCYCLE_RND                        1000

#if LORAWAN_PRINT_DEBUG
#include "lorawan_dbg.h"
#endif


static bool ShouldSendLinkCheck(void) 
{
  static uint32_t counter = 0; 

  if (LORAWAN_LINK_CHECK_INTERVAL <= 0) return 0; 

  if (counter++ > LORAWAN_LINK_CHECK_INTERVAL) 
  {
    counter = 0; 
    return true; 
  }
  return false; 
}



static bool should_request_time = 0; 
static bool RequestTime(void) 
{
  //This sends a time request packet 

  LoRaMacStatus_t status; 
  MlmeReq_t mlmeReq; 
  mlmeReq.Type = MLME_DEVICE_TIME; 
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE; 

  status = LoRaMacMlmeRequest(&mlmeReq); 

#if LORAWAN_PRINT_DEBUG
  printf( "\r\n###### ===== MLME-Request - MLME_DEVICE_TIME ==== ######\r\n" );
  printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif
 DeviceState = DEVICE_STATE_SLEEP;

 should_request_time=0;

 if (status == LORAMAC_STATUS_OK) 
 {
   return 0; 
 }
 else return 1; 


}


/** Send a link check packet */
static bool LinkCheck (void) 
{

  LoRaMacStatus_t status; 
  MlmeReq_t mlmeReq; 
  mlmeReq.Type = MLME_LINK_CHECK; 
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE; 

  status = LoRaMacMlmeRequest(&mlmeReq); 

#if LORAWAN_PRINT_DEBUG
  printf( "\r\n###### ===== MLME-Request - MLME_LINK_CHECK ==== ######\r\n" );
  printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif
 DeviceState = DEVICE_STATE_SLEEP;

 if (status == LORAMAC_STATUS_OK) 
 {
   return 0; 
 }
 return 1; 
}




static void JoinNetwork( void )
{
  LoRaMacStatus_t status;
  MlmeReq_t mlmeReq;
  mlmeReq.Type = MLME_JOIN;
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;
  
  // Starts the join procedure
  status = LoRaMacMlmeRequest( &mlmeReq );
#if LORAWAN_PRINT_DEBUG
  printf( "\r\n###### ===== MLME-Request - MLME_JOIN ==== ######\r\n" );
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
  int buffer_length = first_message_length(&tx); 
  if (buffer_length < 0 || !buffer_length) 
  {

    //do I need to send time? 
    if (should_request_time)
    {
      return RequestTime(); 
    }

    //we have nothing to send right now, consider sending a link check 
    else if (ShouldSendLinkCheck())
    {
      return LinkCheck(); 
    }
    return 0; 
  }

  uint8_t flags = first_message_flags(&tx); 
  uint8_t port = first_message_port(&tx); 
  IsTxConfirmed = !!(flags & LORAWAN_MSG_CONFIRMED); 


  LoRaMacStatus_t cur_status = LoRaMacQueryTxPossible( buffer_length, &txInfo);
  if( cur_status != LORAMAC_STATUS_OK ) {
      printf("Sending empty Data, Loramac didn't return status ok. ");
#if LORAWAN_PRINT_DEBUG
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
        mcpsReq.Req.Unconfirmed.fPort = port;
        mcpsReq.Req.Unconfirmed.fBuffer = buffer;
        mcpsReq.Req.Unconfirmed.fBufferSize = buffer_length;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      } else {
        printf("Sending Confirmed Data.");
        mcpsReq.Type = MCPS_CONFIRMED;
        mcpsReq.Req.Confirmed.fPort = port;
        mcpsReq.Req.Confirmed.fBuffer = buffer;
        mcpsReq.Req.Confirmed.fBufferSize = buffer_length;
        mcpsReq.Req.Confirmed.NbTrials = 8;
        mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      }
    }

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );
    printf( "\r\n###### ===== MCPS-Request ==== ######\r\n" );
#if LORAWAN_PRINT_DEBUG
    printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif 

    if( status == LORAMAC_STATUS_OK )
    {
        //can we pop off the buffer already? 
        // I guess we'll find out! 
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
    (void) context; 
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





int lorawan_process()
{

    //check the interrupts
    int cant_sleep = 1; 

        
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
          mibReq.Param.SystemMaxRxError = LORAWAN_RX_ERROR; 
          LoRaMacMibSetRequestConfirm( &mibReq );

//          mibReq.Type = MIB_MIN_RX_SYMBOLS; 
//          mibReq.Param.MinRxSymbols = 10; 
//          LoRaMacMibSetRequestConfirm( &mibReq );


        
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
        mibReq.Param.SystemMaxRxError = LORAWAN_RX_ERROR;
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
          cant_sleep = 0; 
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

  return cant_sleep;

}

//// CALLBACKS 

static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    printf( "\r\n###### ===== MCPS-Confirm ==== ######\r\n" );
#if LORAWAN_PRINT_DEBUG
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
    printf( "TX PORT     : %d\r\n", first_message_port(&tx));

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

   consume_first_message(&tx); 
}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
#if LORAWAN_PRINT_DEBUG
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
      int len = mcpsIndication->BufferSize; 
      if (have_rx_capacity(len)) 
      {
        uint8_t * dest = 0; 
        int ok = msg_getmem(&rx, len, mcpsIndication->Port, &dest, 0); 
        if (!ok)
        {
          printf("could not get rx mem?\n"); 
          rx.dropped++; 
          memcpy(dest, mcpsIndication->Buffer, len ); 
          msg_push(&rx); 
        }
      }
      else
      {
        printf("!!!! Dropped packet !!!!!"); 
        rx.dropped++; 
      }

    }

    const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    printf( "\r\n###### ===== DOWNLINK FRAME %lu ==== ######\r\n", mcpsIndication->DownLinkCounter );

    printf( "RX WINDOW   : %s\r\n", slotStrings[mcpsIndication->RxSlot] );
    
    printf( "RX PORT     : %d\r\n", mcpsIndication->Port );

    if( mcpsIndication->BufferSize != 0 )
    {
#if LORAWAN_PRINT_DEBUG
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
#if LORAWAN_PRINT_DEBUG
    printf( "\r\n###### ===== MLME-Confirm ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mlmeConfirm->Status] );
#endif
    if( mlmeConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
    }
    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        case MLME_REJOIN_0:
        case MLME_REJOIN_1:
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
          
                joined = 1; 
                // Status is OK, node has joined the network
                DeviceState = DEVICE_STATE_SEND;
            }
            else
            {
                joined = 0; 
                // Join was not successful. Try to join again
                JoinNetwork( );
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
              printf(" Link check succeeded?\n"); 

            }
            else
            {
                joined = 0; 
                DeviceState = DEVICE_STATE_RESTORE;
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
#if LORAWAN_PRINT_DEBUG
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


lwan_state_t lorawan_state(void) 
{

  switch (joined)
  {
    case 0:
      return LORAWAN_JOINING;
    case 1: 
    default: 
      return LORAWAN_READY; 
  }
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
#if LORAWAN_PRINT_DEBUG
   printf( "LoRaMac wasn't properly initialized, error: %s", MacStatusStrings[status] );
#endif
   return status; 
  }

  DeviceState = DEVICE_STATE_RESTORE;

  return 0; 
}



int lorawan_request_datetime() 
{
  should_request_time =1; 
  return 0; 
}


