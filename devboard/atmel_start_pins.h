/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_PINS_H_INCLUDED
#define ATMEL_START_PINS_H_INCLUDED

#include <hal_gpio.h>

// SAMD21 has 8 pin functions

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3
#define GPIO_PIN_FUNCTION_E 4
#define GPIO_PIN_FUNCTION_F 5
#define GPIO_PIN_FUNCTION_G 6
#define GPIO_PIN_FUNCTION_H 7

#define LORA_SPI_MISO GPIO(GPIO_PORTA, 4)
#define LORA_SPI_CS GPIO(GPIO_PORTA, 5)
#define LORA_SPI_MOSI GPIO(GPIO_PORTA, 6)
#define LORA_SPI_SCLK GPIO(GPIO_PORTA, 7)
#define LORA_RESET GPIO(GPIO_PORTA, 8)
#define LORA_DIO2 GPIO(GPIO_PORTA, 9)
#define SPI_FLASH_CS GPIO(GPIO_PORTA, 13)
#define SBC_UART_TX GPIO(GPIO_PORTA, 22)
#define SBC_UART_RX GPIO(GPIO_PORTA, 23)
#define LORA_DIO GPIO(GPIO_PORTB, 6)
#define LORA_DIO1 GPIO(GPIO_PORTB, 7)
#define SPI_FLASH_MISO GPIO(GPIO_PORTB, 16)
#define SPI_FLASH_MOSI GPIO(GPIO_PORTB, 22)
#define SPI_FLASH_SCLK GPIO(GPIO_PORTB, 23)
#define LED GPIO(GPIO_PORTB, 30)

#endif // ATMEL_START_PINS_H_INCLUDED
