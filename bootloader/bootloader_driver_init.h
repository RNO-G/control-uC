/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef DRIVER_INIT_INCLUDED
#define DRIVER_INIT_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_atomic.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_flash.h>
#include <hal_usart_async.h>
#include <hal_spi_m_sync.h>


extern struct flash_descriptor       FLASH;
extern struct usart_async_descriptor SBC_UART;
extern struct spi_m_sync_descriptor  SPI_FLASH;

void FLASH_init(void);
void FLASH_CLOCK_init(void);

void SBC_UART_PORT_init(void);
void SBC_UART_CLOCK_init(void);
void SBC_UART_init(void);

void SPI_FLASH_PORT_init(void);
void SPI_FLASH_CLOCK_init(void);
void SPI_FLASH_init(void);

void FLASH_deinit(void);
void SBC_UART_deinit(void);
void SPI_FLASH_deinit(void);



/**
 * \brief Perform system initialization, initialize pins and clocks for
 * peripherals
 */
void system_init(void);
void system_deinit(void);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_INIT_INCLUDED
