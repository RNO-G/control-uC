#ifndef _spi_flash_h
#define _spi_flash_h


#include "config_block.h" 
#include <stdint.h>


void spi_flash_init(); 



// wake up the flash to make 
void spi_flash_wakeup(); 
void spi_flash_deep_sleep(); 

//1 if busy, 0 if not
int spi_flash_busy(); 


/** This obtains the config block. On boot up, this will read it from SPI flash (or default init it, if we have none)
 * but subsequently it will be cached.
 **/ 
config_block_t * config_block(); 
void config_block_force_read(); 
void config_block_sync(); 



/*Valid slots are 1-4 (since 0 is used for ROM) */ 

int spi_flash_application_seek(int slot, uint32_t offset) ; 
/* This will wait until ready, so for best results, write small amounts at a time on page boundaries */
int spi_flash_application_write(int slot, uint16_t len, const uint8_t* data) ; 


int spi_flash_application_read(int slot, uint16_t len, uint8_t * data) ; 

/* Can take a long time! Only call in bootloader */ 
int spi_flash_application_erase_sync(int slot, int offset,  int nblk); 

/* Returns immediately, but must be called until returns 0. Otherwise returns number of 4k blocks to be erased. */ 
int spi_flash_application_erase_async(int slot,  int offset, int nblk);




/* For debug usage */ 
int spi_flash_raw_read(uint32_t addr, int len, uint8_t * buf); 
uint32_t spi_flash_device_id(); 


#endif
