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
#include <hal_spi_m_sync.h>

#include <hal_i2c_m_async.h>
#include <hal_i2c_m_sync.h>
#include <hal_adc_sync.h>
#include <hal_delay.h>

#ifndef _BOOTLOADER_
#include <hal_calendar.h>
#include <hal_timer.h>
#include <hpl_tc_base.h>
#endif 

//#include "hal_usb_device.h"

#include <hal_wdt.h>

extern struct flash_descriptor       FLASH;
extern struct usart_async_descriptor SBC_UART;
extern struct spi_m_sync_descriptor  SPI_FLASH;

#ifndef _BOOTLOADER_
extern struct spi_m_sync_descriptor  LORA_SPI;
extern struct calendar_descriptor CALENDAR;
extern struct timer_descriptor    LORA_TIMER;
extern struct wdt_descriptor INTERNAL_WATCHDOG;


extern struct usart_async_descriptor LTE_UART;
extern struct usart_async_descriptor SBC_UART_CONSOLE;
extern struct timer_descriptor    SHARED_TIMER;
extern struct adc_sync_descriptor ANALOGIN;
#if USE_SYNCHRONOUS_I2C
extern struct i2c_m_sync_desc I2C;
#else
extern struct i2c_m_async_desc I2C;
#endif //synchrnous i2c
 

#endif  //!bootloader


/**
 * Initialize stuff
 */
void system_init(void);
#ifdef _BOOTLOADER_
void system_deinit(void);
#endif

#endif // DRIVER_INIT_INCLUDED
