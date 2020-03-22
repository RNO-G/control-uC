

#include "spi.h" 
#include "shared/driver_init.h" 



//The radio only uses SpiInOut so that's all we must implement! 


uint16_t SpiInOut(Spi_t *blah, uint16_t out) 
{
  uint8_t rx = 0xff; 
  uint8_t tx = out; 
  struct spi_xfer xfer; 
  xfer.size = 1; 
  xfer.txbuf = &tx; 
  xfer.rxbuf = &rx; 
  spi_m_sync_transfer(&LORA_SPI,&xfer); 
  return rx; 
}

