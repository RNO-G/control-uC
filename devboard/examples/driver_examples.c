/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

static void button_on_PB06_pressed(void)
{
}

static void button_on_PB07_pressed(void)
{
}

static void button_on_PA09_pressed(void)
{
}

/**
 * Example of using EXTERNAL_IRQ_0
 */
void EXTERNAL_IRQ_0_example(void)
{

	ext_irq_register(PIN_PB06, button_on_PB06_pressed);
	ext_irq_register(PIN_PB07, button_on_PB07_pressed);
	ext_irq_register(PIN_PA09, button_on_PA09_pressed);
}

static uint8_t src_data[128];
static uint8_t chk_data[128];
/**
 * Example of using FLASH to read and write Flash main array.
 */
void FLASH_example(void)
{
	uint32_t page_size;
	uint16_t i;

	/* Init source data */
	page_size = flash_get_page_size(&FLASH);

	for (i = 0; i < page_size; i++) {
		src_data[i] = i;
	}

	/* Write data to flash */
	flash_write(&FLASH, 0x3200, src_data, page_size);

	/* Read data from flash */
	flash_read(&FLASH, 0x3200, chk_data, page_size);
}

/**
 * Example of using LORA_SPI to write "Hello World" using the IO abstraction.
 */
static uint8_t example_LORA_SPI[12] = "Hello World!";

void LORA_SPI_example(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&LORA_SPI, &io);

	spi_m_sync_enable(&LORA_SPI);
	io_write(io, example_LORA_SPI, 12);
}

/**
 * Example of using SBC_UART to write "Hello World" using the IO abstraction.
 *
 * Since the driver is asynchronous we need to use statically allocated memory for string
 * because driver initiates transfer and then returns before the transmission is completed.
 *
 * Once transfer has been completed the tx_cb function will be called.
 */

static uint8_t example_SBC_UART[12] = "Hello World!";

static void tx_cb_SBC_UART(const struct usart_async_descriptor *const io_descr)
{
	/* Transfer completed */
}

void SBC_UART_example(void)
{
	struct io_descriptor *io;

	usart_async_register_callback(&SBC_UART, USART_ASYNC_TXC_CB, tx_cb_SBC_UART);
	/*usart_async_register_callback(&SBC_UART, USART_ASYNC_RXC_CB, rx_cb);
	usart_async_register_callback(&SBC_UART, USART_ASYNC_ERROR_CB, err_cb);*/
	usart_async_get_io_descriptor(&SBC_UART, &io);
	usart_async_enable(&SBC_UART);

	io_write(io, example_SBC_UART, 12);
}

/**
 * Example of using SPI_FLASH to write "Hello World" using the IO abstraction.
 */
static uint8_t example_SPI_FLASH[12] = "Hello World!";

void SPI_FLASH_example(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&SPI_FLASH, &io);

	spi_m_sync_enable(&SPI_FLASH);
	io_write(io, example_SPI_FLASH, 12);
}

void delay_example(void)
{
	delay_ms(5000);
}

/**
 * Example of using CALENDAR.
 */
static struct calendar_alarm alarm;

static void alarm_cb(struct calendar_descriptor *const descr)
{
	/* alarm expired */
}

void CALENDAR_example(void)
{
	struct calendar_date date;
	struct calendar_time time;

	calendar_enable(&CALENDAR);

	date.year  = 2000;
	date.month = 12;
	date.day   = 31;

	time.hour = 12;
	time.min  = 59;
	time.sec  = 59;

	calendar_set_date(&CALENDAR, &date);
	calendar_set_time(&CALENDAR, &time);

	alarm.cal_alarm.datetime.time.sec = 4;
	alarm.cal_alarm.option            = CALENDAR_ALARM_MATCH_SEC;
	alarm.cal_alarm.mode              = REPEAT;

	calendar_set_alarm(&CALENDAR, &alarm, alarm_cb);
}

static struct timer_task LORA_TIMER_task1, LORA_TIMER_task2;

/**
 * Example of using LORA_TIMER.
 */
static void LORA_TIMER_task1_cb(const struct timer_task *const timer_task)
{
}

static void LORA_TIMER_task2_cb(const struct timer_task *const timer_task)
{
}

void LORA_TIMER_example(void)
{
	LORA_TIMER_task1.interval = 100;
	LORA_TIMER_task1.cb       = LORA_TIMER_task1_cb;
	LORA_TIMER_task1.mode     = TIMER_TASK_REPEAT;
	LORA_TIMER_task2.interval = 200;
	LORA_TIMER_task2.cb       = LORA_TIMER_task2_cb;
	LORA_TIMER_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&LORA_TIMER, &LORA_TIMER_task1);
	timer_add_task(&LORA_TIMER, &LORA_TIMER_task2);
	timer_start(&LORA_TIMER);
}

/**
 * Example of using INTERNAL_WATCHDOG.
 */
void INTERNAL_WATCHDOG_example(void)
{
	uint32_t clk_rate;
	uint16_t timeout_period;

	clk_rate       = 1000;
	timeout_period = 4096;
	wdt_set_timeout_period(&INTERNAL_WATCHDOG, clk_rate, timeout_period);
	wdt_enable(&INTERNAL_WATCHDOG);
}
