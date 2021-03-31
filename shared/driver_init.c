/*
 */

#include "shared/driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#ifndef _BOOTLOADER_ 
#include <hal_ext_irq.h>
#endif
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

#ifndef _BOOTLOADER_
#include <hpl_adc_base.h>

/*! The buffer size for USART */
#define LTE_UART_BUFFER_SIZE 256

/*! The buffer size for USART */
#define SBC_UART_CONSOLE_BUFFER_SIZE 256

#endif

/*! The buffer size for USART */
#define SBC_UART_BUFFER_SIZE 256

struct usart_async_descriptor SBC_UART;
struct spi_m_sync_descriptor  SPI_FLASH;

#ifndef _BOOTLOADER_
struct spi_m_sync_descriptor  LORA_SPI;
struct timer_descriptor       LORA_TIMER;

struct usart_async_descriptor SBC_UART_CONSOLE;
struct usart_async_descriptor LTE_UART;
struct adc_sync_descriptor ANALOGIN;
struct timer_descriptor       SHARED_TIMER;

static uint8_t LTE_UART_buffer[LTE_UART_BUFFER_SIZE];
static uint8_t SBC_UART_CONSOLE_buffer[SBC_UART_CONSOLE_BUFFER_SIZE];
#endif 
static uint8_t SBC_UART_buffer[SBC_UART_BUFFER_SIZE];


struct flash_descriptor FLASH;

#if USE_SYNCHRONOUS_I2C
struct i2c_m_sync_desc I2C;
#else
struct i2c_m_async_desc I2C;
#endif

#ifndef _BOOTLOADER_
struct calendar_descriptor CALENDAR;
struct wdt_descriptor INTERNAL_WATCHDOG;
#endif 


#ifndef _BOOTLOADER_
static void ANALOGIN_PORT_init(void)
{

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN1, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN1, PINMUX_PA03B_ADC_AIN1);

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN12, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN12, PINMUX_PB04B_ADC_AIN12);

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN13, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN13, PINMUX_PB05B_ADC_AIN13);

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN14, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN14, PINMUX_PB06B_ADC_AIN14);

	// Disable digital pin circuitry
	gpio_set_pin_direction(AIN15, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(AIN15, PINMUX_PB07B_ADC_AIN15);
}

static void ANALOGIN_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, ADC);
	_gclk_enable_channel(ADC_GCLK_ID, CONF_GCLK_ADC_SRC);
}

static void ANALOGIN_init(void)
{
	ANALOGIN_CLOCK_init();
	ANALOGIN_PORT_init();
	adc_sync_init(&ANALOGIN, ADC, (void *)NULL);
}

static void EXT_IRQ_INIT(void)
{
	_gclk_enable_channel(EIC_GCLK_ID, CONF_GCLK_EIC_SRC);

#ifndef USE_RADIO_DEBUG
	gpio_set_pin_direction(GPIO1, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(GPIO1, GPIO_PULL_OFF);
	gpio_set_pin_function(GPIO1, PINMUX_PA04A_EIC_EXTINT4);
#endif


	gpio_set_pin_direction(LORA_DIO2, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(LORA_DIO2, GPIO_PULL_OFF);
	gpio_set_pin_function(LORA_DIO2, PINMUX_PA05A_EIC_EXTINT5);

	gpio_set_pin_direction(LORA_DIO1, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(LORA_DIO1, GPIO_PULL_OFF);
	gpio_set_pin_function(LORA_DIO1, PINMUX_PA09A_EIC_EXTINT9);

	gpio_set_pin_direction(LORA_DIO, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(LORA_DIO, GPIO_PULL_OFF);
	gpio_set_pin_function(LORA_DIO, PINMUX_PA10A_EIC_EXTINT10);

	ext_irq_init();
}
#endif


/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
static void SBC_UART_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
static void SBC_UART_PORT_init()
{

	gpio_set_pin_function(SBC_UART_TX, PINMUX_PA06D_SERCOM0_PAD2);

	gpio_set_pin_function(SBC_UART_RX, PINMUX_PA07D_SERCOM0_PAD3);
}

/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
static void SBC_UART_init(void)
{
	SBC_UART_CLOCK_init();
	usart_async_init(&SBC_UART, SERCOM0, SBC_UART_buffer, SBC_UART_BUFFER_SIZE, (void *)NULL);
	SBC_UART_PORT_init();
}

#ifndef _BOOTLOADER_
/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
static void LTE_UART_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM1);
	_gclk_enable_channel(SERCOM1_GCLK_ID_CORE, CONF_GCLK_SERCOM1_CORE_SRC);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
static void LTE_UART_PORT_init()
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
static void LTE_UART_init(void)
{
	LTE_UART_CLOCK_init();
	usart_async_init(&LTE_UART, SERCOM1, LTE_UART_buffer, LTE_UART_BUFFER_SIZE, (void *)NULL);
	LTE_UART_PORT_init();
}
#endif

static void SPI_FLASH_PORT_init(void)
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

static void SPI_FLASH_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM2);
	_gclk_enable_channel(SERCOM2_GCLK_ID_CORE, CONF_GCLK_SERCOM2_CORE_SRC);
}

static void SPI_FLASH_init(void)
{
	SPI_FLASH_CLOCK_init();
	spi_m_sync_init(&SPI_FLASH, SERCOM2);
	SPI_FLASH_PORT_init();
}

#ifndef _BOOTLOADER_
/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
static void SBC_UART_CONSOLE_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
	_gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
static void SBC_UART_CONSOLE_PORT_init()
{

	gpio_set_pin_function(SBC_UART_CONSOLE_TX, PINMUX_PA20D_SERCOM3_PAD2);

	gpio_set_pin_function(SBC_UART_CONSOLE_RX, PINMUX_PA21D_SERCOM3_PAD3);
}

/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
static void SBC_UART_CONSOLE_init(void)
{
	SBC_UART_CONSOLE_CLOCK_init();
	usart_async_init(&SBC_UART_CONSOLE, SERCOM3, SBC_UART_CONSOLE_buffer, SBC_UART_CONSOLE_BUFFER_SIZE, (void *)NULL);
	SBC_UART_CONSOLE_PORT_init();
}

static void LORA_SPI_PORT_init(void)
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

static void LORA_SPI_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM4);
	_gclk_enable_channel(SERCOM4_GCLK_ID_CORE, CONF_GCLK_SERCOM4_CORE_SRC);
}

static void LORA_SPI_init(void)
{
	LORA_SPI_CLOCK_init();
	spi_m_sync_init(&LORA_SPI, SERCOM4);
	LORA_SPI_PORT_init();
}

static void I2C_PORT_init(void)
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

static void I2C_PORT_deinit(void)
{

	gpio_set_pin_function(I2C_SDA, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(I2C_SCL, GPIO_PIN_FUNCTION_OFF);
}

static void I2C_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM5);
	_gclk_enable_channel(SERCOM5_GCLK_ID_CORE, CONF_GCLK_SERCOM5_CORE_SRC);
	_gclk_enable_channel(SERCOM5_GCLK_ID_SLOW, CONF_GCLK_SERCOM5_SLOW_SRC);
}

void I2C_init(void)
{
	I2C_CLOCK_init();
#ifdef USE_SYNCHRONOUS_I2C
	i2c_m_sync_init(&I2C, SERCOM5);
#else
	i2c_m_async_init(&I2C, SERCOM5);
#endif
	I2C_PORT_init();
}


void I2C_deinit(void) 
{
#ifdef USE_SYNCHRONOUS_I2C
	i2c_m_sync_deinit(&I2C);
#else
	i2c_m_async_deinit(&I2C);
#endif
	
	I2C_PORT_deinit();

}


static void SHARED_TIMER_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, TC4);
	_gclk_enable_channel(TC4_GCLK_ID, CONF_GCLK_TC4_SRC);

	timer_init(&SHARED_TIMER, TC4, _tc_get_timer());

  //go ahead and start the timer
  timer_start(&SHARED_TIMER); 
}


#endif  //not bootloader


///// common to all 
static void FLASH_CLOCK_init(void)
{

	_pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
}

static void FLASH_init(void)
{
	FLASH_CLOCK_init();
	flash_init(&FLASH, NVMCTRL);
}



static void delay_driver_init(void)
{
	delay_init(SysTick);
}

#ifndef _BOOTLOADER_

static void CALENDAR_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBA, RTC);
	_gclk_enable_channel(RTC_GCLK_ID, CONF_GCLK_RTC_SRC);
}

static void CALENDAR_init(void)
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
#endif




void system_init(void)
{
	init_mcu();

	gpio_set_pin_level(SPI_FLASH_CS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_FLASH_CS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SPI_FLASH_CS, GPIO_PIN_FUNCTION_OFF);



#ifndef _BOOTLOADER_
	gpio_set_pin_level(LORA_RESET,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_RESET, GPIO_DIRECTION_IN);

	gpio_set_pin_function(LORA_RESET, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB13

	gpio_set_pin_level(LORA_SPI_CS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(LORA_SPI_CS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LORA_SPI_CS, GPIO_PIN_FUNCTION_OFF);


	gpio_set_pin_level(VICOR_EN, true);
	gpio_set_pin_direction(VICOR_EN, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(VICOR_EN, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(NALERT, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NALERT, GPIO_PULL_OFF);
	gpio_set_pin_function(NALERT, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(LTE_REG_EN, false);
	gpio_set_pin_direction(LTE_REG_EN, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(LTE_REG_EN, GPIO_PIN_FUNCTION_OFF);


	gpio_set_pin_level(WATCHDOG, false);
	gpio_set_pin_direction(WATCHDOG, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(WATCHDOG, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(HEATER_FET_CNTRL, false);
	gpio_set_pin_direction(HEATER_FET_CNTRL, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(HEATER_FET_CNTRL, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(LED_GREEN, false);
	gpio_set_pin_direction(LED_GREEN, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(LED_GREEN, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(SBC_SOFT_RESET, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(SBC_SOFT_RESET,GPIO_PULL_OFF); 
	gpio_set_pin_function(SBC_SOFT_RESET, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(SBC_BOOT_CONTROL, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(SBC_BOOT_CONTROL, GPIO_PULL_OFF);
	gpio_set_pin_function(SBC_BOOT_CONTROL, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(LTE_UART_ENABLE, false); 
	gpio_set_pin_direction(LTE_UART_ENABLE, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(LTE_UART_ENABLE, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(LTE_DTR, false);
	gpio_set_pin_direction(LTE_DTR, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(LTE_DTR, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(FAULT_5V, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(FAULT_5V, GPIO_PULL_OFF);
	gpio_set_pin_function(FAULT_5V, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(LTE_ON_OFF, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(LTE_ON_OFF, GPIO_PULL_OFF);
	gpio_set_pin_function(LTE_ON_OFF, GPIO_PIN_FUNCTION_OFF);

	ANALOGIN_init();
	SBC_UART_CONSOLE_init();
	LTE_UART_init();
  SHARED_TIMER_init(); 
#endif  // bootloader

	// Set pin direction to input
	gpio_set_pin_direction(GPIO3, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(GPIO3, GPIO_PULL_OFF);
	gpio_set_pin_function(GPIO3, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(GPIO2, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(GPIO2, GPIO_PULL_OFF);
	gpio_set_pin_function(GPIO2, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(GPIO1, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(GPIO1, GPIO_PULL_OFF);
	gpio_set_pin_function(GPIO1, GPIO_PIN_FUNCTION_OFF);

	FLASH_init();
	SBC_UART_init();
	SPI_FLASH_init();

	delay_driver_init();

#ifndef _BOOTLOADER_
	LORA_SPI_init();


	CALENDAR_init();

	LORA_TIMER_init();

  EXT_IRQ_INIT(); 

	INTERNAL_WATCHDOG_init();
#endif
}


#ifdef _BOOTLOADER_

static void SPI_FLASH_deinit(void)
{
  _pm_disable_bus_clock(PM_BUS_APBC, SERCOM2); 
	spi_m_sync_deinit(&SPI_FLASH); 
  gpio_set_pin_function(SPI_FLASH_SCLK, GPIO_PIN_FUNCTION_OFF);
  gpio_set_pin_function(SPI_FLASH_MOSI, GPIO_PIN_FUNCTION_OFF);
  gpio_set_pin_function(SPI_FLASH_MISO, GPIO_PIN_FUNCTION_OFF);
}

static void SBC_UART_deinit(void) 
{
	_pm_disable_bus_clock(PM_BUS_APBC, SERCOM0);
	usart_async_deinit(&SBC_UART); 
	gpio_set_pin_function(SBC_UART_TX, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(SBC_UART_RX, GPIO_PIN_FUNCTION_OFF);

}

static void FLASH_deinit(void)
{
	_pm_disable_bus_clock(PM_BUS_APBB, NVMCTRL);
	flash_deinit(&FLASH);
}


void system_deinit(void) 
{
  SPI_FLASH_deinit(); 
  FLASH_deinit(); 
  SBC_UART_deinit(); 
}
#endif 
