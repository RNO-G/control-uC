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

struct spi_m_sync_descriptor SPI_FLASH;
struct spi_m_sync_descriptor LORA_SPI;

struct flash_descriptor FLASH;

struct usart_sync_descriptor SBC_UART;

struct usart_sync_descriptor LTE_UART;

struct usart_sync_descriptor SBC_UART_CONSOLE;

struct i2c_m_sync_desc I2C;

struct calendar_descriptor CALENDAR;

struct wdt_descriptor INTERNAL_WATCHDOG;

void FLASH_CLOCK_init(void)
{

	_pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
}

void FLASH_init(void)
{
	FLASH_CLOCK_init();
	flash_init(&FLASH, NVMCTRL);
}

void SBC_UART_PORT_init(void)
{

	gpio_set_pin_function(PA06, PINMUX_PA06D_SERCOM0_PAD2);

	gpio_set_pin_function(PA07, PINMUX_PA07D_SERCOM0_PAD3);
}

void SBC_UART_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

void SBC_UART_init(void)
{
	SBC_UART_CLOCK_init();
	usart_sync_init(&SBC_UART, SERCOM0, (void *)NULL);
	SBC_UART_PORT_init();
}

void LTE_UART_PORT_init(void)
{

	gpio_set_pin_function(PA16, PINMUX_PA16C_SERCOM1_PAD0);

	gpio_set_pin_function(PA17, PINMUX_PA17C_SERCOM1_PAD1);

	gpio_set_pin_function(PA18, PINMUX_PA18C_SERCOM1_PAD2);

	gpio_set_pin_function(PA19, PINMUX_PA19C_SERCOM1_PAD3);
}

void LTE_UART_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM1);
	_gclk_enable_channel(SERCOM1_GCLK_ID_CORE, CONF_GCLK_SERCOM1_CORE_SRC);
}

void LTE_UART_init(void)
{
	LTE_UART_CLOCK_init();
	usart_sync_init(&LTE_UART, SERCOM1, (void *)NULL);
	LTE_UART_PORT_init();
}

void SPI_FLASH_PORT_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(PA12, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PA12,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PA12, PINMUX_PA12C_SERCOM2_PAD0);

	gpio_set_pin_level(PA14,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PA14, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PA14, PINMUX_PA14C_SERCOM2_PAD2);

	gpio_set_pin_level(PA15,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PA15, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PA15, PINMUX_PA15C_SERCOM2_PAD3);
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

void SBC_UART_CONSOLE_PORT_init(void)
{

	gpio_set_pin_function(PA20, PINMUX_PA20D_SERCOM3_PAD2);

	gpio_set_pin_function(PA21, PINMUX_PA21D_SERCOM3_PAD3);
}

void SBC_UART_CONSOLE_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
	_gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
}

void SBC_UART_CONSOLE_init(void)
{
	SBC_UART_CONSOLE_CLOCK_init();
	usart_sync_init(&SBC_UART_CONSOLE, SERCOM3, (void *)NULL);
	SBC_UART_CONSOLE_PORT_init();
}

void LORA_SPI_PORT_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(PB12, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PB12,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PB12, PINMUX_PB12C_SERCOM4_PAD0);

	gpio_set_pin_level(PB14,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PB14, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PB14, PINMUX_PB14C_SERCOM4_PAD2);

	gpio_set_pin_level(PB15,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PB15, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PB15, PINMUX_PB15C_SERCOM4_PAD3);
}

void LORA_SPI_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM4);
	_gclk_enable_channel(SERCOM4_GCLK_ID_CORE, CONF_GCLK_SERCOM4_CORE_SRC);
}

void LORA_SPI_init(void)
{
	LORA_SPI_CLOCK_init();
	spi_m_sync_init(&LORA_SPI, SERCOM4);
	LORA_SPI_PORT_init();
}

void I2C_PORT_init(void)
{

	gpio_set_pin_pull_mode(PB30,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PB30, PINMUX_PB30D_SERCOM5_PAD0);

	gpio_set_pin_pull_mode(PB31,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PB31, PINMUX_PB31D_SERCOM5_PAD1);
}

void I2C_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM5);
	_gclk_enable_channel(SERCOM5_GCLK_ID_CORE, CONF_GCLK_SERCOM5_CORE_SRC);
	_gclk_enable_channel(SERCOM5_GCLK_ID_SLOW, CONF_GCLK_SERCOM5_SLOW_SRC);
}

void I2C_init(void)
{
	I2C_CLOCK_init();
	i2c_m_sync_init(&I2C, SERCOM5);
	I2C_PORT_init();
}

void CALENDAR_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBA, RTC);
	_gclk_enable_channel(RTC_GCLK_ID, CONF_GCLK_RTC_SRC);
}

void CALENDAR_init(void)
{
	CALENDAR_CLOCK_init();
	calendar_init(&CALENDAR, RTC);
}

void USB_DBG_PORT_init(void)
{

	gpio_set_pin_direction(PA24,
	                       // <y> Pin direction
	                       // <id> pad_direction
	                       // <GPIO_DIRECTION_OFF"> Off
	                       // <GPIO_DIRECTION_IN"> In
	                       // <GPIO_DIRECTION_OUT"> Out
	                       GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA24,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_pull_mode(PA24,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PA24,
	                      // <y> Pin function
	                      // <id> pad_function
	                      // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
	                      // <PINMUX_PA24G_USB_DM"> Auto
	                      // <GPIO_PIN_FUNCTION_OFF"> Off
	                      // <GPIO_PIN_FUNCTION_A"> A
	                      // <GPIO_PIN_FUNCTION_B"> B
	                      // <GPIO_PIN_FUNCTION_C"> C
	                      // <GPIO_PIN_FUNCTION_D"> D
	                      // <GPIO_PIN_FUNCTION_E"> E
	                      // <GPIO_PIN_FUNCTION_F"> F
	                      // <GPIO_PIN_FUNCTION_G"> G
	                      // <GPIO_PIN_FUNCTION_H"> H
	                      PINMUX_PA24G_USB_DM);

	gpio_set_pin_direction(PA25,
	                       // <y> Pin direction
	                       // <id> pad_direction
	                       // <GPIO_DIRECTION_OFF"> Off
	                       // <GPIO_DIRECTION_IN"> In
	                       // <GPIO_DIRECTION_OUT"> Out
	                       GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA25,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_pull_mode(PA25,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PA25,
	                      // <y> Pin function
	                      // <id> pad_function
	                      // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
	                      // <PINMUX_PA25G_USB_DP"> Auto
	                      // <GPIO_PIN_FUNCTION_OFF"> Off
	                      // <GPIO_PIN_FUNCTION_A"> A
	                      // <GPIO_PIN_FUNCTION_B"> B
	                      // <GPIO_PIN_FUNCTION_C"> C
	                      // <GPIO_PIN_FUNCTION_D"> D
	                      // <GPIO_PIN_FUNCTION_E"> E
	                      // <GPIO_PIN_FUNCTION_F"> F
	                      // <GPIO_PIN_FUNCTION_G"> G
	                      // <GPIO_PIN_FUNCTION_H"> H
	                      PINMUX_PA25G_USB_DP);
}

/* The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
 * for low speed and full speed operation. */
#if (CONF_GCLK_USB_FREQUENCY > (48000000 + 48000000 / 400)) || (CONF_GCLK_USB_FREQUENCY < (48000000 - 48000000 / 400))
#warning USB clock should be 48MHz ~ 0.25% clock, check your configuration!
#endif

void USB_DBG_CLOCK_init(void)
{

	_pm_enable_bus_clock(PM_BUS_APBB, USB);
	_pm_enable_bus_clock(PM_BUS_AHB, USB);
	_gclk_enable_channel(USB_GCLK_ID, CONF_GCLK_USB_SRC);
}

void USB_DBG_init(void)
{
	USB_DBG_CLOCK_init();
	usb_d_init();
	USB_DBG_PORT_init();
}

void INTERNAL_WATCHDOG_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBA, WDT);
	_gclk_enable_channel(WDT_GCLK_ID, CONF_GCLK_WDT_SRC);
}

void INTERNAL_WATCHDOG_init(void)
{
	INTERNAL_WATCHDOG_CLOCK_init();
	wdt_init(&INTERNAL_WATCHDOG, WDT);
}

void system_init(void)
{
	init_mcu();

	// GPIO on PA00

	gpio_set_pin_function(XTAL_IN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA01

	gpio_set_pin_function(XTAL_OUT, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA03

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN1, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN1, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA05

	gpio_set_pin_level(SBC_UART_EN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SBC_UART_EN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SBC_UART_EN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA08

	gpio_set_pin_level(LORA_DIO2,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_DIO2, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_DIO2, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA09

	gpio_set_pin_level(LORA_DIO1,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_DIO1, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_DIO1, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA10

	gpio_set_pin_level(LORA_DIO,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_DIO, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_DIO, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA11

	gpio_set_pin_level(LORA_RESET,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_RESET, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_RESET, GPIO_PIN_FUNCTION_OFF);

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

	// GPIO on PA22

	gpio_set_pin_level(VICOR_EN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(VICOR_EN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(VICOR_EN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA23

	// Set pin direction to input
	gpio_set_pin_direction(ALERT, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(ALERT,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(ALERT, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA27

	gpio_set_pin_level(LTE_UART_EN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LTE_UART_EN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LTE_UART_EN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA28

	gpio_set_pin_level(WATCHDOG,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(WATCHDOG, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(WATCHDOG, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB02

	gpio_set_pin_level(SBC_BOOT_CTRL,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SBC_BOOT_CTRL, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SBC_BOOT_CTRL, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB04

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN12, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN12, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB05

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN13, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN13, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB06

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN14, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN14, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB09

	gpio_set_pin_level(SBC_SOFT_RESET,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SBC_SOFT_RESET, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SBC_SOFT_RESET, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB13

	gpio_set_pin_level(LORA_SPI_CS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_SPI_CS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_SPI_CS, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB16

	gpio_set_pin_level(LTE_DTR,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LTE_DTR, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LTE_DTR, GPIO_PIN_FUNCTION_OFF);

	FLASH_init();

	SBC_UART_init();

	LTE_UART_init();

	SPI_FLASH_init();

	SBC_UART_CONSOLE_init();

	LORA_SPI_init();

	I2C_init();

	CALENDAR_init();

	USB_DBG_init();

	INTERNAL_WATCHDOG_init();
}
