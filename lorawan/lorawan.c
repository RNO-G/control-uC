
#include "config/lorawan_config.h" 

#include "shared/printf.h" 
#include "application/time.h" 
#include "shared/spi_flash.h" 
#include "application/lowpower.h" 


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
static uint8_t tx_buffer[LORAWAN_BUFFER_SIZE]; 
static uint16_t tx_offsets[LORAWAN_MAX_MESSAGES]; 
static uint8_t tx_flags[LORAWAN_MAX_MESSAGES]; 
static uint8_t tx_ports[LORAWAN_MAX_MESSAGES]; 

static uint8_t rx_buffer[LORAWAN_BUFFER_SIZE]; 
static uint16_t rx_offsets[LORAWAN_MAX_MESSAGES]; 
static uint8_t rx_flags[LORAWAN_MAX_MESSAGES]; 
static uint8_t rx_ports[LORAWAN_MAX_MESSAGES]; 

static volatile int joined = 0; 
static int last_cycle_or_join =0; 
static int last_sent = -1; 
static int last_received = -1; 
static int last_queued = -1; 
static int last_link_check = -1; 
static int last_join_request = -1; 
static int last_link_request = -1; 
static int process_pending = 0; 
static int last_time_request = -1; 
static int join_time; 
static int rssi = 0; 
static int snr = 0; 
static int time_check = 0; 

struct msg_buffer 
{
  uint8_t * buffer; 
  uint16_t * offsets;
  uint16_t used; 
  uint8_t n_messages; 
  volatile uint8_t got_mem; 
  uint8_t * flags; 
  uint8_t * ports; 
  uint32_t dropped; 
  uint32_t count; 
  //a tx message is first pushed, then queued queued, then consumed. rx messages are just pushed and consumed. 
  //NOTE: when consuming a message it should be unqueued! 
  volatile uint32_t pushed; 
  volatile uint32_t queued; 
  volatile uint32_t consumed; 
}; 

static struct msg_buffer tx = {.buffer = tx_buffer, .offsets = tx_offsets, .used = 0, .n_messages = 0, .flags = tx_flags, .ports = tx_ports, .dropped = 0, .count=0};
static struct msg_buffer rx = {.buffer = rx_buffer, .offsets = rx_offsets, .used = 0, .n_messages = 0, .flags = rx_flags, .ports = rx_ports, .dropped = 0, .count=0};

void lorawan_stats(rno_g_lora_stats_t * stats) 
{
  if (!stats) return; 

  stats->when = get_time(); 
  stats->uptime = uptime(); 
  stats->tx = tx.count; 
  stats->rx = rx.count; 
  stats->tx_dropped = tx.dropped; 
  stats->rx_dropped = rx.dropped; 
  stats->last_recv = last_received; 
  stats->last_check = last_link_check; 
  stats->last_sent = last_sent; 
  stats->join_time = join_time; 
  stats->rssi = rssi; 
  stats->snr = snr; 
}

static inline  uint8_t * first_message(struct msg_buffer * b) {
  
  int offs = 0; 
  CRITICAL_SECTION_BEGIN() ; 
  offs = b->offsets[b->queued+b->consumed] ; 
  CRITICAL_SECTION_END() ; 

  return b->buffer + offs; 

}

static int first_message_to_consume_length(struct msg_buffer * b)
{
  int len; 
  CRITICAL_SECTION_BEGIN() ; 
  len = b->consumed == 0 ? -1 : b->n_messages == 1? b->used :  b->offsets[1] ; 
  CRITICAL_SECTION_END() ; 

  return len; 
}


static int first_message_length(struct msg_buffer * b)
{
  int nmessages; 
  CRITICAL_SECTION_BEGIN() ; 
  nmessages = b->n_messages-(b->queued + b->consumed); 
  int end = nmessages == 1 ? b->used : b->offsets[b->queued + b->consumed + 1]; 
  int len = nmessages == 0 ? -1 :  end - b->offsets[b->queued+b->consumed] ; 
  CRITICAL_SECTION_END() ; 

  return len; 
}

static uint8_t first_message_flags(struct msg_buffer * b) 
{
  int imessage; 
  CRITICAL_SECTION_BEGIN() ; 
  imessage = (b->queued + b->consumed); 
  CRITICAL_SECTION_END() ; 
  if (imessage < 0) return 0; 
  else return b->flags[imessage]; 
}

static uint8_t first_message_port(struct msg_buffer * b) 
{
  int imessage; 
  CRITICAL_SECTION_BEGIN() ; 
  imessage = b->queued + b->consumed; 
  CRITICAL_SECTION_END() ; 
 
  return imessage < 0 ? 0 : b->ports[imessage]; 
}

static void consume_first_message(struct msg_buffer *b)
{

  //the simplest case
  if (b->n_messages == 0)
  {
    return; 
  }

  //the simple case
  else if (b->n_messages == 1) 
  {
    b->n_messages=0 ;
    b->used = 0; 
    memset(b->offsets,0,sizeof(*b->offsets)*LORAWAN_MAX_MESSAGES); 

    CRITICAL_SECTION_BEGIN();
    b->consumed--; 
    CRITICAL_SECTION_END();
  }

  CRITICAL_SECTION_BEGIN();
  if (!b->queued) //can't move memory from underneath it! 
  {
    int len = first_message_to_consume_length(b); 
    if (len >= 0)
    {
      ASSERT(b->used >= len); 
      b->used-=len; 
      memmove(b->buffer, b->buffer+len, b->used); 
      b->n_messages--; 

      if (b->n_messages) 
      {
        //NOTE: these 3 can be circular buffers... change later. 
        memmove(b->flags, b->flags+1, b->n_messages); 
        memmove(b->ports, b->ports+1, b->n_messages); 
        memmove(b->offsets, b->offsets+1, b->n_messages*sizeof(*b->offsets)); 
        for (int i = 0; i < b->n_messages; i++) b->offsets[i]-=len; 
      }
      b->consumed--; 
    }
  }
  CRITICAL_SECTION_END();
}



static int have_tx_capacity(int len)
{

 // CRITICAL_SECTION_BEGIN() ; 
  int have_room = (len+tx.used < LORAWAN_BUFFER_SIZE) && (tx.n_messages < LORAWAN_MAX_MESSAGES);
 // CRITICAL_SECTION_END() ; 

  return have_room; 
}
static int have_rx_capacity(int len)
{
 // CRITICAL_SECTION_BEGIN() ; 
  int have_room = (len+rx.used < LORAWAN_BUFFER_SIZE) && (rx.n_messages < LORAWAN_MAX_MESSAGES);
  //CRITICAL_SECTION_END() ; 

  return have_room;
}





int msg_getmem(struct msg_buffer *b, uint8_t len, uint8_t port, uint8_t **msg, uint8_t flags) 
{
  int got_mem = 0; 

  //see if we need to clear anything 
  while (b->consumed && !b->queued)
  {
    consume_first_message(b); 
  }

  got_mem = b->got_mem; 

  if (got_mem) 
  {
    b->dropped++; 
  }
  else
  {
    b->got_mem = 1; 
    b->offsets[b->n_messages] = b->used; 
    b->flags[b->n_messages] = flags; 
    b->ports[b->n_messages] = port; 
    *msg = b->buffer+b->used; 
    b->used+= len; 
  }
  return  got_mem ? -2 : 0; 
}


int msg_push(struct msg_buffer *b) 
{
  if (!b->got_mem) 
  {
    return -1;
  }
  b->pushed++; 
  b->n_messages++; 
  b->got_mem = 0; 
  return 0; 

}

int lorawan_tx_getmem(uint8_t len, uint8_t port, uint8_t **msg, uint8_t flags) 
{
  if (!have_tx_capacity(len))
  {
    //try to consume 
    while (tx.consumed && !tx.queued)
    {
      consume_first_message(&tx); 
    }

    if (!have_tx_capacity(len)) 
    {
      tx.dropped++; 
      return -1; 
    }
  }
  return msg_getmem(&tx,len,port,msg,flags);
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
  if (!rx.n_messages || rx.n_messages <= rx.consumed) return 0; 
  if (len) *len = first_message_length(&rx); 
  if (port) *port = first_message_port(&rx); 
  if (flags) *flags = first_message_flags(&rx); 
  if (msg) *msg= first_message(&rx); 
  return rx.n_messages; 
}

int lorawan_rx_pop() 
{
  if (!rx.pushed) return -1; 
  CRITICAL_SECTION_BEGIN();
  rx.pushed--; 
  rx.consumed++; 
  CRITICAL_SECTION_END();
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
 CRITICAL_SECTION_BEGIN();
 rx.consumed++; 
 CRITICAL_SECTION_END();
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
static uint16_t channelsMask[] = LORAWAN_CHANNEL_MASK; 
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


#if LORAWAN_PRINT_DEBUG
#include "lorawan_dbg.h"
#endif



static uint32_t link_check_counter = 0; 
static uint32_t failed_link_checks =0; 

static bool ShouldSendLinkCheck(void) 
{

  if (LORAWAN_LINK_CHECK_INTERVAL <= 0) return false; 

  if (link_check_counter++ > LORAWAN_LINK_CHECK_INTERVAL) 
  {
    link_check_counter-=50; 
    return true; 
  }
  return false; 
}



static volatile int  should_request_time = 0; 
static void RequestTime(void) 
{
  //This sends a time request packet 

  LoRaMacStatus_t status; 
  MlmeReq_t mlmeReq; 
  mlmeReq.Type = MLME_DEVICE_TIME; 
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE; 

  last_time_request = uptime(); 
  status = LoRaMacMlmeRequest(&mlmeReq); 

#if LORAWAN_PRINT_DEBUG
  printf( "\r\n###### ===== MLME-Request - MLME_DEVICE_TIME ==== ######\r\n" );
  printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif
 DeviceState = DEVICE_STATE_SLEEP;


 if (status == LORAMAC_STATUS_OK) 
 {
   should_request_time=0;
 }
}


/** Send a link check packet */
static void LinkCheck (void) 
{

  LoRaMacStatus_t status; 
  MlmeReq_t mlmeReq; 
  mlmeReq.Type = MLME_LINK_CHECK; 
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE; 
  last_link_request = uptime(); 
  printf("#LORA: Sending link check.\r\n"); 
  status = LoRaMacMlmeRequest(&mlmeReq); 

#if LORAWAN_PRINT_DEBUG
  printf( "\r\n###### ===== MLME-Request - MLME_LINK_CHECK ==== ######\r\n" );
  printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif

  if (status != LORAMAC_STATUS_OK) 
  {
    //force retry 
    link_check_counter = LORAWAN_LINK_CHECK_INTERVAL+1; 
  }
}




static void JoinNetwork( void )
{
  LoRaMacStatus_t status;
  MlmeReq_t mlmeReq;
  mlmeReq.Type = MLME_JOIN;
  mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;
  last_join_request = uptime(); 
  
  // Starts the join procedure
  status = LoRaMacMlmeRequest( &mlmeReq );
#if LORAWAN_PRINT_DEBUG
  printf( "\r\n###### ===== MLME-Request - MLME_JOIN ==== ######\r\n" );
  printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
#endif

  if( status == LORAMAC_STATUS_OK ) {
#if LORAWAN_PRINT_DEBUG
      printf( "#LORA: ###### ===== JOINING ==== ######\r\n" );
#endif 
      int up = uptime(); 
      last_cycle_or_join = up; 
      printf("#LORA: Joining... (uptime: %d)\r\n", up); 
      DeviceState = DEVICE_STATE_SLEEP;
  } else 
  {  
    DeviceState = DEVICE_STATE_CYCLE;
  }
}

// Indicates if the node is sending confirmed or unconfirmed messages
static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;


/* Send the next thing in our buffer, I guess ? */ 
static volatile int sent_empty = 0; 
static uint8_t * lora_buffer = 0; 
static bool SendFrame( void ) { 

  if (tx.queued) return true; 
  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  int buffer_length = first_message_length(&tx); 
  if (buffer_length < 0 || !buffer_length) 
  {
    //do I need to send time? 
    if (should_request_time)
    {
      RequestTime(); 
    }

    //we have nothing to send right now, consider sending a link check 
    else if (ShouldSendLinkCheck())
    {
      LinkCheck(); 
    }
    return 0; 
  }

  uint8_t flags = first_message_flags(&tx); 
  uint8_t port = first_message_port(&tx); 
  IsTxConfirmed = !!(flags & LORAWAN_MSG_CONFIRMED); 


  LoRaMacStatus_t cur_status = LoRaMacQueryTxPossible( buffer_length, &txInfo);
  if( cur_status != LORAMAC_STATUS_OK ) {
      printf("#LORA: Sending empty Data, Loramac returned status %s.\r\n ", MacStatusStrings[cur_status] );
      
      // Send empty frame in order to flush MAC commands
      mcpsReq.Type = MCPS_UNCONFIRMED;
      mcpsReq.Req.Unconfirmed.fPort = 0;
      mcpsReq.Req.Unconfirmed.fBuffer = NULL;
      mcpsReq.Req.Unconfirmed.fBufferSize = 0;
      mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      sent_empty = 1; 
    } else { 

     
      last_queued = uptime(); 
      CRITICAL_SECTION_BEGIN()
      lora_buffer = first_message(&tx); 
      tx.pushed--; 
      tx.queued++; 
      CRITICAL_SECTION_END()


      if( IsTxConfirmed == false ) {
#if LORAWAN_PRINT_DEBUG
        printf("Sending unconfirmed Data.");
#endif
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fPort = port;
        mcpsReq.Req.Unconfirmed.fBuffer = lora_buffer;
        mcpsReq.Req.Unconfirmed.fBufferSize = buffer_length;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      } else {
#if LORAWAN_PRINT_DEBUG
        printf("Sending Confirmed Data.");
#endif
        mcpsReq.Type = MCPS_CONFIRMED;
        mcpsReq.Req.Confirmed.fPort = port;
        mcpsReq.Req.Confirmed.fBuffer = lora_buffer;
        mcpsReq.Req.Confirmed.fBufferSize = buffer_length;
        mcpsReq.Req.Confirmed.NbTrials = 8;
        mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      }
    }

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );
#if LORAWAN_PRINT_DEBUG
    printf( "\r\n###### ===== MCPS-Request ==== ######\r\n" );
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
    process_pending = 0; 

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
    else
    {
      DeviceState = DEVICE_STATE_RESTORE; 

      printf("#LORA: OnTxNextPacketTimerEvent returned %d\n, restoring", status); 
    }
}



static int next_lora_stats = 20; 
int lorawan_process(int up)
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
          if( ( devEui[3] == 0 ) && ( devEui[4] == 0 ) && ( devEui[5] == 0 ) && ( devEui[6] == 0 ) && ( devEui[7] == 0 ) ) {
            uint16_t unique;
            BoardGetUniqueId( &unique );
            devEui[6] = (unique >> 8) & 0xff;
            devEui[7] = unique & 0xff;
          }

          mibReq.Type = MIB_DEV_EUI;
          mibReq.Param.DevEui = devEui;
          LoRaMacMibSetRequestConfirm( &mibReq );
            
          mibReq.Type = MIB_JOIN_EUI;
          mibReq.Param.JoinEui = joinEui;
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_CHANNELS_TX_POWER; 
          mibReq.Param.ChannelsTxPower = LORAWAN_TX_POWER; 
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_CHANNELS_DEFAULT_TX_POWER; 
          mibReq.Param.ChannelsDefaultTxPower = LORAWAN_TX_POWER; 
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR; 
          mibReq.Param.SystemMaxRxError = LORAWAN_RX_ERROR; 
          LoRaMacMibSetRequestConfirm( &mibReq );

          mibReq.Type = MIB_CHANNELS_MASK; 
          mibReq.Param.ChannelsMask[0] = channelsMask[0]; 
          mibReq.Param.ChannelsMask[1] = channelsMask[1]; 
          mibReq.Param.ChannelsMask[2] = channelsMask[2]; 
          mibReq.Param.ChannelsMask[3] = channelsMask[3]; 
          mibReq.Param.ChannelsMask[4] = channelsMask[4]; 
          LoRaMacMibSetRequestConfirm( &mibReq );



 //         mibReq.Type = MIB_MIN_RX_SYMBOLS; 
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
#if LORAWAN_PRINT_DEBUG
        printf( "DevEui      : %02X", mibReq.Param.DevEui[0] );
        for( int i = 1; i < 8; i++ )
        {
          printf( "-%02X", mibReq.Param.DevEui[i] );
        }
        printf( "\r\n" );
#endif
        mibReq.Type = MIB_JOIN_EUI;
        LoRaMacMibGetRequestConfirm( &mibReq );
#if LORAWAN_PRINT_DEBUG
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

#endif
        JoinNetwork( );    
        break;
      }

      case DEVICE_STATE_SEND: {
        if(NextTx == true) {
            NextTx = SendFrame( );
            cant_sleep = 1; 
        }
        DeviceState = DEVICE_STATE_CYCLE;
        break;
      }


      case DEVICE_STATE_CYCLE: {
        DeviceState = DEVICE_STATE_SLEEP;
        TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
//        if (low_power_mode) TxDutyCycleTime >>=4; 

        // Schedule next packet transmission
        TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
        TimerStart( &TxNextPacketTimer );
        last_cycle_or_join = up; 
        break;
      }

      case DEVICE_STATE_SLEEP: {
      
        CRITICAL_SECTION_BEGIN( ); 
        if( IsMacProcessPending == 1 ) {
          IsMacProcessPending = 0; // Clear flag and prevent MCU to go into low power modes.
          process_pending = 1; //our persistent flag until sleep ends 
        } else {      
          if (!process_pending && !tx.queued && last_sent + 4 < up && last_join_request + 6 < up && last_link_request + 3 < up  && last_time_request + 3 < up) 
          {
            cant_sleep = 0;  
          //BoardLowPowerHandler( ); // The MCU wakes up through events
          }
        }
        CRITICAL_SECTION_END( );

        if (up > last_cycle_or_join+60) 
        {
          printf("#LORA: slept too long  %d, %d?!?\r\n", up, last_cycle_or_join) ; 
          OnTxNextPacketTimerEvent(NULL); 
        }
        break;
      }

      default: {
        DeviceState = DEVICE_STATE_START;
        break;
      }
      
    } // end of switch


    //see if we need to sent lora stas
   if (joined && (up > next_lora_stats) && have_tx_capacity(sizeof(rno_g_lora_stats_t)))
   {
       rno_g_lora_stats_t stats;
       lorawan_stats(&stats); 
       lorawan_tx_copy(sizeof(stats), RNO_G_MSG_LORA_STATS, (uint8_t*) &stats, 0); 

       int interval = low_power_mode ? config_block()->app_cfg.lora_stats_interval_low_power_mode : config_block()->app_cfg.lora_stats_interval ; 
       if (interval < 10) interval = 10; 
       next_lora_stats = up + interval;
       cant_sleep =1; 
   }

   if (joined && ( up >= time_check) )
   {
     int have_time = get_time() > 1000000000; 
     should_request_time = 1; 
     int delay_in_secs = have_time ? config_block()->app_cfg.timesync_interval :  low_power_mode ?  60 : 15; 
     time_check+= delay_in_secs ;
     cant_sleep=1; 
   }


  return cant_sleep;
}


int lorawan_sched_timesync() 
{
  should_request_time = 1; 
  return 0; 
}

//// CALLBACKS 

static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
#if LORAWAN_PRINT_DEBUG
    printf( "\r\n###### ===== MCPS-Confirm ==== ######\r\n" );
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

#if LORAWAN_PRINT_DEBUG
    MibRequestConfirm_t mibGet;
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );

    printf( "\r\n###### ===== UPLINK FRAME %lu ==== ######\r\n", mcpsConfirm->UpLinkCounter );
    printf( "\r\n" );

    printf( "CLASS       : %c\r\n", "ABC"[mibReq.Param.Class] );
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
#endif

  if (sent_empty) 
  {
    CRITICAL_SECTION_BEGIN();
    if (tx.queued) 
    {
      tx.pushed++; 
      tx.queued--; 
    }
    CRITICAL_SECTION_END();
 
    sent_empty = 0; 
  }
  else
  {
    CRITICAL_SECTION_BEGIN();
    tx.consumed++; 
    tx.queued--; 
    CRITICAL_SECTION_END();
    tx.count++; 
  }
  last_sent = uptime(); 
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

    int confirmed = 0; 
    switch( mcpsIndication->McpsIndication ) {
    case MCPS_UNCONFIRMED: { 
      break;
    }
    case MCPS_CONFIRMED: { 
      confirmed = 1; 
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

      if (!have_rx_capacity(len))
      {
        //try to make some room 
        while (rx.consumed) 
        {
          consume_first_message(&rx); 
        }
      }

      if (have_rx_capacity(len)) 
      {
        uint8_t * dest = 0; 
        int ok = msg_getmem(&rx, len, mcpsIndication->Port, &dest, confirmed); 
        if (!ok)
        {
          memcpy(dest, mcpsIndication->Buffer, len ); 
          rx.count++; 
          msg_push(&rx); 
        }
        else
        {
          rx.dropped++; 
          printf("#LORA: could not get rx mem?\n"); 
        }
      }
      else
      {
        printf("#LORA: !!!! Dropped packet !!!!!"); 
        rx.dropped++; 
      }

    }
#if LORAWAN_PRINT_DEBUG

    const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    printf( "\r\n###### ===== DOWNLINK FRAME %lu ==== ######\r\n", mcpsIndication->DownLinkCounter );

    printf( "RX WINDOW   : %s\r\n", slotStrings[mcpsIndication->RxSlot] );
    
    printf( "RX PORT     : %d\r\n", mcpsIndication->Port );

    if( mcpsIndication->BufferSize != 0 )
    {
        printf( "RX DATA     : \r\n" );
        PrintHexBuffer( mcpsIndication->Buffer, mcpsIndication->BufferSize );
    }

    printf( "\r\n" );
    printf( "DATA RATE   : DR_%d\r\n", mcpsIndication->RxDatarate );
    printf( "RX RSSI     : %d\r\n", mcpsIndication->Rssi );
    printf( "RX SNR      : %d\r\n", mcpsIndication->Snr );
    printf( "\r\n" );
#endif
    rssi = mcpsIndication->Rssi; 
    snr = mcpsIndication->Snr; 
    last_received = uptime(); 
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
              printf("#LORA: Joined! (uptime: %u)\r\n", uptime()); 

#if LORAWAN_PRINT_DEBUG
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
#endif
          
                joined = 1; 
                join_time = uptime(); 
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
            if( mlmeConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
            {
              failed_link_checks++; 
              printf("#LORA:  Link check failed (%d/%d)!\r\n", failed_link_checks, LORAWAN_MAX_FAILED_LINK_CHECKS); 

              if (failed_link_checks > LORAWAN_MAX_FAILED_LINK_CHECKS) 
              {
                printf("#LORA: rejoining"); 
                failed_link_checks = 0; 
                joined = 0; 
                DeviceState = DEVICE_STATE_JOIN;
 
              }
           }
            else
            {
              printf("#LORA: Link check succeeded.\r\n"); 
              failed_link_checks = 0; 
              link_check_counter = 0; 
              last_link_check = uptime(); 
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




int lorawan_init(int initial) 
{

  if (initial) 
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
  }
  else
  {
    //reset the eui so we recreate it properly , skipping the IEE_OUI
    for (unsigned i = 3; i < sizeof(devEui); i++) devEui[i] = 0; 
  }

  DeviceState = DEVICE_STATE_RESTORE;

  return 0; 
}




int lorawan_tx_push() 
{
  int ret = msg_push(&tx); 
  OnTxNextPacketTimerEvent(0); 
  return ret; 
}

int lorawan_reset() 
{
  SX1272Reset(); 
  return lorawan_init(0); 
}


int lorawan_last_recv() 
{
  return last_received; 

}
