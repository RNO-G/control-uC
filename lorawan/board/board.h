#ifndef _lorawan_board_h
#define _lorawan_board_h


#include <atmel_start_pins.h>
#include <stdint.h>
#include "utilities.h"

#define BOARD_TCXO_WAKEUP_TIME                      0

#define RADIO_RESET LORA_RESET
#define RADIO_MOSI LORA_SPI_MOSI
#define RADIO_MISO LORA_SPI_MISO
#define RADIO_SCLK LORA_SPI_SCLK
#define RADIO_NSS LORA_SPI_CS
#define RADIO_DIO_0 LORA_DIO
#define RADIO_DIO_1 LORA_DIO1
#define RADIO_DIO_2 LORA_DIO2

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 16 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint16_t *id );

/*!
 * \brief Get the board version
 *
 * \retval value  Version
 */
Version_t BoardGetVersion( void );



#endif
