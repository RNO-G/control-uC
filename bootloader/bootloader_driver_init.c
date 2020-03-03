/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

/*! The buffer size for USART */
#define SBC_UART_BUFFER_SIZE 16

/*! The buffer size for USART */
#define LTE_UART_BUFFER_SIZE 16

/*! The buffer size for USART */
#define SBC_UART_CONSOLE_BUFFER_SIZE 16

struct usart_async_descriptor SBC_UART;
struct spi_m_sync_descriptor  SPI_FLASH;

static uint8_t SBC_UART_buffer[SBC_UART_BUFFER_SIZE];

struct flash_descriptor FLASH;



void FLASH_CLOCK_init(void)
{

	_pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
}


void FLASH_init(void)
{
	FLASH_CLOCK_init();
	flash_init(&FLASH, NVMCTRL);
}

void FLASH_deinit(void)
{
	_pm_disable_bus_clock(PM_BUS_APBB, NVMCTRL);
	flash_deinit(&FLASH);
}

/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
void SBC_UART_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

void SBC_UART_CLOCK_deinit()
{

//	_gclk_disable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}
/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
void SBC_UART_PORT_init()
{

	gpio_set_pin_function(SBC_UART_TX, PINMUX_PA06D_SERCOM0_PAD2);

	gpio_set_pin_function(SBC_UART_RX, PINMUX_PA07D_SERCOM0_PAD3);
}


/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
void SBC_UART_init(void)
{
	SBC_UART_CLOCK_init();
	usart_async_init(&SBC_UART, SERCOM0, SBC_UART_buffer, SBC_UART_BUFFER_SIZE, (void *)NULL);
	SBC_UART_PORT_init();
}


void SBC_UART_deinit(void) 
{
	_pm_disable_bus_clock(PM_BUS_APBC, SERCOM0);
	usart_async_deinit(&SBC_UART); 
	gpio_set_pin_function(SBC_UART_TX, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(SBC_UART_RX, GPIO_PIN_FUNCTION_OFF);

}

void SPI_FLASH_PORT_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(SPI_FLASH_MISO, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(SPI_FLASH_MISO,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(SPI_FLASH_MISO, PINMUX_PA12C_SERCOM2_PAD0);

	gpio_set_pin_level(SPI_FLASH_MOSI,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_FLASH_MOSI, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SPI_FLASH_MOSI, PINMUX_PA14C_SERCOM2_PAD2);

	gpio_set_pin_level(SPI_FLASH_SCLK,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_FLASH_SCLK, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SPI_FLASH_SCLK, PINMUX_PA15C_SERCOM2_PAD3);
}

void SPI_FLASH_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM2);
	_gclk_enable_channel(SERCOM2_GCLK_ID_CORE, CONF_GCLK_SERCOM2_CORE_SRC);
}

void SPI_FLASH_init(void)
{
	SPI_FLASH_CLOCK_init();
	spi_m_sync_init(&SPI_FLASH, SERCOM2);
	SPI_FLASH_PORT_init();
}

void SPI_FLASH_deinit(void)
{
  _pm_disable_bus_clock(PM_BUS_APBC, SERCOM2); 
	spi_m_sync_deinit(&SPI_FLASH); 
  gpio_set_pin_function(SPI_FLASH_SCLK, GPIO_PIN_FUNCTION_OFF);
  gpio_set_pin_function(SPI_FLASH_MOSI, GPIO_PIN_FUNCTION_OFF);
  gpio_set_pin_function(SPI_FLASH_MISO, GPIO_PIN_FUNCTION_OFF);
}



void system_deinit() 
{
  SPI_FLASH_deinit(); 
  FLASH_deinit(); 
  SBC_UART_deinit(); 
}

void system_init(void)
{
	init_mcu();

	// GPIO on PA03

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN1, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN1, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA04

	// Set pin direction to input
	gpio_set_pin_direction(GPIO1, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(GPIO1,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(GPIO1, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA05

	// Set pin direction to input
	gpio_set_pin_direction(GPIO0, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(GPIO0,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(GPIO0, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA13
	gpio_set_pin_level(SPI_FLASH_CS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_FLASH_CS, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(SPI_FLASH_CS, GPIO_PIN_FUNCTION_OFF);


	FLASH_init();

	SBC_UART_init();

	SPI_FLASH_init();

}
