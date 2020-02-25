#ifndef PIN_NAME_BOARD_H
#define PIN_NAME_BOARD_H

/*/  
 * this only defines the pins we actually use 
 * and it of course names them the normal way 
 */ 

#include "atmel_start_pins.h" 


#define MCU_PINS \
  LORA_PIN_DIO0 = LORA_DIO,       \
  LORA_PIN_DIO1 = LORA_DIO1,      \
  LORA_PIN_DIO2 = LORA_DIO2,      \
  LORA_PIN_RESET = LORA_RESET,    \
  LORA_PIN_MISO = LORA_SPI_MISO,  \
  LORA_PIN_MOSI = LORA_SPI_MOSI,  \
  LORA_PIN_CS = LORA_SPI_CS,      \
  LORA_PIN_SCLK = LORA_SPI_SCLK

#endif 
