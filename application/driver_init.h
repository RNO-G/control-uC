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

#include <peripheral_clk_config.h>
#include <hpl_gclk_base.h>
#include "config/config.h" 
#include <hal_atomic.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_sleep.h>

#include <hal_flash.h>

#include <hal_usart_async.h>
#include <hal_usart_async.h>
#include <hal_spi_m_sync.h>
#include <hal_usart_async.h>
#include <hal_spi_m_sync.h>


#ifndef _DEVBOARD_
#include <hal_i2c_m_async.h>
#include <hal_i2c_m_sync.h>
#include <hal_adc_sync.h>
#endif

#include <hal_delay.h>

#include <hal_calendar.h>
#include <hal_timer.h>
#include <hpl_tc_base.h>

//#include "hal_usb_device.h"

#include <hal_wdt.h>

extern struct flash_descriptor       FLASH;
extern struct usart_async_descriptor SBC_UART;
extern struct spi_m_sync_descriptor  SPI_FLASH;
extern struct spi_m_sync_descriptor  LORA_SPI;
extern struct calendar_descriptor CALENDAR;
extern struct timer_descriptor    LORA_TIMER;
extern struct wdt_descriptor INTERNAL_WATCHDOG;


#ifndef _DEVBOARD_
extern struct usart_async_descriptor LTE_UART;
extern struct usart_async_descriptor SBC_UART_CONSOLE;
extern struct timer_descriptor    SHARED_TIMER;
extern struct adc_sync_descriptor ANALOGIN;
#if USE_SYNCHRONOUS_I2C
extern struct i2c_m_sync_desc I2C;
#else
extern struct i2c_m_async_desc I2C;
#endif
#endif 



void FLASH_init(void);
void FLASH_CLOCK_init(void);

void SBC_UART_PORT_init(void);
void SBC_UART_CLOCK_init(void);
void SBC_UART_init(void);

void SPI_FLASH_PORT_init(void);
void SPI_FLASH_CLOCK_init(void);
void SPI_FLASH_init(void);


#ifndef _DEVBOARD_
void LTE_UART_PORT_init(void);
void LTE_UART_CLOCK_init(void);
void LTE_UART_init(void);

void SBC_UART_CONSOLE_PORT_init(void);
void SBC_UART_CONSOLE_CLOCK_init(void);
void SBC_UART_CONSOLE_init(void);

void I2C_PORT_init(void);
void I2C_CLOCK_init(void);
void I2C_init(void);

void ADC_PORT_init(void);
void ADC_CLOCK_init(void);
void ADC_init(void);


#endif

void LORA_SPI_PORT_init(void);
void LORA_SPI_CLOCK_init(void);
void LORA_SPI_init(void);

void delay_driver_init(void);

void CALENDAR_CLOCK_init(void);
void CALENDAR_init(void);

void INTERNAL_WATCHDOG_CLOCK_init(void);
void INTERNAL_WATCHDOG_init(void);

/**
 * \brief Perform system initialization, initialize pins and clocks for
 * peripherals
 */
void system_init(void);

#endif // DRIVER_INIT_INCLUDED
