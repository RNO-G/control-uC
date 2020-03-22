
#include "radio.h"
#include "board.h" 
#include "sx1272-board.h"
#include <hal_ext_irq.h>
#include <hal_gpio.h>
#include "delay.h"
#include "shared/printf.h" 
#include "shared/driver_init.h" 


static uint8_t SX1272GetPaSelect( uint32_t channel );

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1272Init,
    SX1272GetStatus,
    SX1272SetModem,
    SX1272SetChannel,
    SX1272IsChannelFree,
    SX1272Random,
    SX1272SetRxConfig,
    SX1272SetTxConfig,
    SX1272CheckRfFrequency,
    SX1272GetTimeOnAir,
    SX1272Send,
    SX1272SetSleep,
    SX1272SetStby,
    SX1272SetRx,
    SX1272StartCad,
    SX1272SetTxContinuousWave,
    SX1272ReadRssi,
    SX1272Write,
    SX1272Read,
    SX1272WriteBuffer,
    SX1272ReadBuffer,
    SX1272SetMaxPayloadLength,
    SX1272SetPublicNetwork,
    SX1272GetWakeupTime,
    NULL, // void ( *IrqProcess )( void )
    NULL, // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
    NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};


void SX1272IoInit( void )
{
  // most things are set up in driver_init, so we'll jsut do a few things here! 

    SX1272.Spi.Nss.pin = LORA_SPI_CS; 
    SX1272.DIO0.pin = LORA_DIO; 
    SX1272.DIO1.pin = LORA_DIO1; 
    SX1272.DIO2.pin = LORA_DIO2; 
    SX1272.DIO3.pin = NC; 
    SX1272.DIO4.pin = NC; 
    SX1272.DIO5.pin = NC; 

    spi_m_sync_set_baudrate(&LORA_SPI,50000); 
    spi_m_sync_enable(&LORA_SPI); 
}

static void Dio0IrqHandler( void );
static void Dio1IrqHandler( void );
static void Dio2IrqHandler( void );

static Gpio_t *DioIrqs[] = {
    &SX1272.DIO0,
    &SX1272.DIO1,
    &SX1272.DIO2,
    NULL,
    NULL,
    NULL
};

static ext_irq_cb_t ExtIrqHandlers[] = {
    Dio0IrqHandler,
    Dio1IrqHandler,
    Dio2IrqHandler,
    NULL,
    NULL,
    NULL
};

static void DioIrqHandlerProcess( uint8_t index )
{
    if( ( DioIrqs[index] != NULL ) && ( DioIrqs[index]->IrqHandler != NULL ) )
    {
        DioIrqs[index]->IrqHandler( DioIrqs[index]->Context );
    }
}

static volatile int ndio0; 

static void Dio0IrqHandler( void )
{
    ndio0++; 
    DioIrqHandlerProcess( 0 );
}

static void Dio1IrqHandler( void )
{
    DioIrqHandlerProcess( 1 );
}

static void Dio2IrqHandler( void )
{
    DioIrqHandlerProcess( 2 );
}

static void IoIrqInit( uint8_t index, DioIrqHandler *irqHandler )
{
    DioIrqs[index]->IrqHandler = irqHandler;
    ext_irq_register( DioIrqs[index]->pin, ExtIrqHandlers[index] );
}

void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{
    for( int8_t i = 0; i < 3; i++ )
    {
        IoIrqInit( i, irqHandlers[i] );
    }
}

void SX1272IoDeInit( void )
{
  ///there ain't no deiniting this 
}

void SX1272IoDbgInit( void )
{
}

void SX1272IoTcxoInit( void )
{
    // No TCXO component available on this board design.
}

void SX1272SetBoardTcxo( uint8_t state )
{
}

uint32_t SX1272GetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

/** TODO: this is blocking. */ 
void SX1272Reset( void )
{
    // Enables the TCXO if available on the board design
    SX1272SetBoardTcxo( true );

    // Set RESET pin to 0
    GpioInit( &SX1272.Reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    // Wait 1 ms
    delay_ms( 1 );

    // Configure RESET as input
    GpioInit( &SX1272.Reset, RADIO_RESET, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    // Wait 6 ms
    delay_ms( 6 );
}

void SX1272SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = SX1272Read( REG_PACONFIG );
    paDac = SX1272Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1272GetPaSelect( SX1272.Settings.Channel );

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power < -1 )
        {
            power = -1;
        }
        if( power > 14 )
        {
            power = 14;
        }
        paConfig = ( paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
    }
    SX1272Write( REG_PACONFIG, paConfig );
    SX1272Write( REG_PADAC, paDac );

}

static uint8_t SX1272GetPaSelect( uint32_t channel )
{
    return RF_PACONFIG_PASELECT_RFO;
}

void SX1272SetAntSwLowPower( bool status )
{
    // No antenna switch available.
    // Just control the TCXO if available.
    if( RadioIsActive != status )
    {
        RadioIsActive = status;
    }
}

void SX1272SetAntSw( uint8_t opMode )
{
    // No antenna switch available
}

bool SX1272CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

