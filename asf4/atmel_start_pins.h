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

#define AIN1 GPIO(GPIO_PORTA, 3)
#define GPIO1 GPIO(GPIO_PORTA, 4)
#define GPIO0 GPIO(GPIO_PORTA, 5)
#define SBC_UART_TX GPIO(GPIO_PORTA, 6)
#define SBC_UART_RX GPIO(GPIO_PORTA, 7)
#define LORA_DIO2 GPIO(GPIO_PORTA, 8)
#define LORA_DIO1 GPIO(GPIO_PORTA, 9)
#define LORA_DIO GPIO(GPIO_PORTA, 10)
#define LORA_RESET GPIO(GPIO_PORTA, 11)
#define SPI_FLASH_MISO GPIO(GPIO_PORTA, 12)
#define SPI_FLASH_CS GPIO(GPIO_PORTA, 13)
#define SPI_FLASH_MOSI GPIO(GPIO_PORTA, 14)
#define SPI_FLASH_SCLK GPIO(GPIO_PORTA, 15)
#define LTE_UART_TX GPIO(GPIO_PORTA, 16)
#define LTE_UART_RX GPIO(GPIO_PORTA, 17)
#define LTE_UART_RTS GPIO(GPIO_PORTA, 18)
#define LTE_UART_CTS GPIO(GPIO_PORTA, 19)
#define SBC_UART_CONSOLE_TX GPIO(GPIO_PORTA, 20)
#define SBC_UART_CONSOLE_RX GPIO(GPIO_PORTA, 21)
#define VICOR_EN GPIO(GPIO_PORTA, 22)
#define NALERT GPIO(GPIO_PORTA, 23)
#define LTE_REG_EN GPIO(GPIO_PORTA, 27)
#define WATCHDOG GPIO(GPIO_PORTA, 28)
#define HEATER_FET_CNTRL GPIO(GPIO_PORTB, 0)
#define LED_GREEN GPIO(GPIO_PORTB, 1)
#define SBC_SOFT_RESET GPIO(GPIO_PORTB, 3)
#define AIN12 GPIO(GPIO_PORTB, 4)
#define AIN13 GPIO(GPIO_PORTB, 5)
#define AIN14 GPIO(GPIO_PORTB, 6)
#define AIN15 GPIO(GPIO_PORTB, 7)
#define GPIO3 GPIO(GPIO_PORTB, 8)
#define GPIO2 GPIO(GPIO_PORTB, 9)
#define SBC_BOOT_CONTROL GPIO(GPIO_PORTB, 10)
#define LTE_UART_ENABLE GPIO(GPIO_PORTB, 11)
#define LORA_SPI_MISO GPIO(GPIO_PORTB, 12)
#define LORA_SPI_CS GPIO(GPIO_PORTB, 13)
#define LORA_SPI_MOSI GPIO(GPIO_PORTB, 14)
#define LORA_SPI_SCLK GPIO(GPIO_PORTB, 15)
#define LTE_DTR GPIO(GPIO_PORTB, 16)
#define FAULT_5V GPIO(GPIO_PORTB, 22)
#define LTE_ON_OFF GPIO(GPIO_PORTB, 23)
#define I2C_SDA GPIO(GPIO_PORTB, 30)
#define I2C_SCL GPIO(GPIO_PORTB, 31)

#endif // ATMEL_START_PINS_H_INCLUDED
