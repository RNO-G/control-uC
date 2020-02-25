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
struct usart_async_descriptor LTE_UART;
struct spi_m_sync_descriptor  SPI_FLASH;
struct usart_async_descriptor SBC_UART_CONSOLE;
struct spi_m_sync_descriptor  LORA_SPI;
struct timer_descriptor       LORA_TIMER;

static uint8_t SBC_UART_buffer[SBC_UART_BUFFER_SIZE];
static uint8_t LTE_UART_buffer[LTE_UART_BUFFER_SIZE];
static uint8_t SBC_UART_CONSOLE_buffer[SBC_UART_CONSOLE_BUFFER_SIZE];

struct flash_descriptor FLASH;

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

/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
void LTE_UART_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM1);
	_gclk_enable_channel(SERCOM1_GCLK_ID_CORE, CONF_GCLK_SERCOM1_CORE_SRC);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
void LTE_UART_PORT_init()
{

	gpio_set_pin_function(LTE_UART_TX, PINMUX_PA16C_SERCOM1_PAD0);

	gpio_set_pin_function(LTE_UART_RX, PINMUX_PA17C_SERCOM1_PAD1);

	gpio_set_pin_function(LTE_UART_RTS, PINMUX_PA18C_SERCOM1_PAD2);

	gpio_set_pin_function(LTE_UART_CTS, PINMUX_PA19C_SERCOM1_PAD3);
}

/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
void LTE_UART_init(void)
{
	LTE_UART_CLOCK_init();
	usart_async_init(&LTE_UART, SERCOM1, LTE_UART_buffer, LTE_UART_BUFFER_SIZE, (void *)NULL);
	LTE_UART_PORT_init();
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

/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
void SBC_UART_CONSOLE_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
	_gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
void SBC_UART_CONSOLE_PORT_init()
{

	gpio_set_pin_function(SBC_UART_CONSOLE_TX, PINMUX_PA20D_SERCOM3_PAD2);

	gpio_set_pin_function(SBC_UART_CONSOLE_RX, PINMUX_PA21D_SERCOM3_PAD3);
}

/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
void SBC_UART_CONSOLE_init(void)
{
	SBC_UART_CONSOLE_CLOCK_init();
	usart_async_init(&SBC_UART_CONSOLE, SERCOM3, SBC_UART_CONSOLE_buffer, SBC_UART_CONSOLE_BUFFER_SIZE, (void *)NULL);
	SBC_UART_CONSOLE_PORT_init();
}

void LORA_SPI_PORT_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(LORA_SPI_MISO, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(LORA_SPI_MISO,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(LORA_SPI_MISO, PINMUX_PB12C_SERCOM4_PAD0);

	gpio_set_pin_level(LORA_SPI_MOSI,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_SPI_MOSI, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_SPI_MOSI, PINMUX_PB14C_SERCOM4_PAD2);

	gpio_set_pin_level(LORA_SPI_SCLK,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_SPI_SCLK, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_SPI_SCLK, PINMUX_PB15C_SERCOM4_PAD3);
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

	gpio_set_pin_pull_mode(I2C_SDA,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(I2C_SDA, PINMUX_PB30D_SERCOM5_PAD0);

	gpio_set_pin_pull_mode(I2C_SCL,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(I2C_SCL, PINMUX_PB31D_SERCOM5_PAD1);
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

void delay_driver_init(void)
{
	delay_init(SysTick);
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

/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void LORA_TIMER_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, TC3);
	_gclk_enable_channel(TC3_GCLK_ID, CONF_GCLK_TC3_SRC);

	timer_init(&LORA_TIMER, TC3, _tc_get_timer());
}

void USB_DBG_PORT_init(void)
{

	gpio_set_pin_direction(USB_DMINUS,
	                       // <y> Pin direction
	                       // <id> pad_direction
	                       // <GPIO_DIRECTION_OFF"> Off
	                       // <GPIO_DIRECTION_IN"> In
	                       // <GPIO_DIRECTION_OUT"> Out
	                       GPIO_DIRECTION_OUT);

	gpio_set_pin_level(USB_DMINUS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_pull_mode(USB_DMINUS,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(USB_DMINUS,
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

	gpio_set_pin_direction(USB_DPLUS,
	                       // <y> Pin direction
	                       // <id> pad_direction
	                       // <GPIO_DIRECTION_OFF"> Off
	                       // <GPIO_DIRECTION_IN"> In
	                       // <GPIO_DIRECTION_OUT"> Out
	                       GPIO_DIRECTION_OUT);

	gpio_set_pin_level(USB_DPLUS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_pull_mode(USB_DPLUS,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(USB_DPLUS,
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
	gpio_set_pin_direction(NALERT, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(NALERT,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(NALERT, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA27

	gpio_set_pin_level(LTE_REG_EN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LTE_REG_EN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LTE_REG_EN, GPIO_PIN_FUNCTION_OFF);

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

	// GPIO on PB00

	gpio_set_pin_level(HEATER_FET_CNTRL,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(HEATER_FET_CNTRL, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(HEATER_FET_CNTRL, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB01

	gpio_set_pin_level(LED_GREEN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LED_GREEN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LED_GREEN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB03

	gpio_set_pin_level(SBC_SOFT_RESET,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SBC_SOFT_RESET, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SBC_SOFT_RESET, GPIO_PIN_FUNCTION_OFF);

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

	// GPIO on PB07

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN15, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN15, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB08

	// Set pin direction to input
	gpio_set_pin_direction(GPIO3, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(GPIO3,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(GPIO3, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB09

	// Set pin direction to input
	gpio_set_pin_direction(GPIO2, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(GPIO2,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(GPIO2, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB10

	gpio_set_pin_level(SBC_BOOT_CONTROL,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SBC_BOOT_CONTROL, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SBC_BOOT_CONTROL, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB11

	gpio_set_pin_level(LTE_UART_ENABLE,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LTE_UART_ENABLE, GPIO_PIN_FUNCTION_OFF);

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

	// GPIO on PB22

	// Set pin direction to input
	gpio_set_pin_direction(FAULT_5V, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(FAULT_5V,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(FAULT_5V, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB23

	gpio_set_pin_level(LTE_ON_OFF,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LTE_ON_OFF, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LTE_ON_OFF, GPIO_PIN_FUNCTION_OFF);

	FLASH_init();

	SBC_UART_init();
	LTE_UART_init();

	SPI_FLASH_init();
	SBC_UART_CONSOLE_init();

	LORA_SPI_init();

	I2C_init();

	delay_driver_init();

	CALENDAR_init();

	LORA_TIMER_init();

	USB_DBG_init();

	INTERNAL_WATCHDOG_init();
}
