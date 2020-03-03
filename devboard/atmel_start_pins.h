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

#define PA04 GPIO(GPIO_PORTA, 4)
#define LORA_SPI_CS GPIO(GPIO_PORTA, 5)
#define PA06 GPIO(GPIO_PORTA, 6)
#define PA07 GPIO(GPIO_PORTA, 7)
#define SPI_FLASH_CS GPIO(GPIO_PORTA, 13)
#define PA16 GPIO(GPIO_PORTA, 16)
#define PA17 GPIO(GPIO_PORTA, 17)
#define GPI00 GPIO(GPIO_PORTA, 20)
#define PA22 GPIO(GPIO_PORTA, 22)
#define PA23 GPIO(GPIO_PORTA, 23)
#define PA24 GPIO(GPIO_PORTA, 24)
#define PA25 GPIO(GPIO_PORTA, 25)
#define LORA_DIO1 GPIO(GPIO_PORTB, 6)
#define LORA_DIO2 GPIO(GPIO_PORTB, 7)
#define LORA_DIO0 GPIO(GPIO_PORTB, 8)
#define LORA_SPI_RESET GPIO(GPIO_PORTB, 9)
#define PB16 GPIO(GPIO_PORTB, 16)
#define PB22 GPIO(GPIO_PORTB, 22)
#define PB23 GPIO(GPIO_PORTB, 23)
#define LED GPIO(GPIO_PORTB, 30)

#endif // ATMEL_START_PINS_H_INCLUDED
