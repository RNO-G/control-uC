#include "shared/spi_flash.h" 
#include "hal_spi_m_sync.h" 
#include "shared/driver_init.h" 

//#include "linker/map.h" 



/* 
 *Code interfacing with the AT25DF081 flash chip
  Cosmin Deaconu <cozzyd@kicp.uchicago.edu> 
 */ 



// registers
const uint8_t  READ_ARRAY = 0x0b; 
const uint8_t  ERASE_4KB = 0x20;
const uint8_t  ERASE_32KB = 0x52;
const uint8_t  ERASE_64KB = 0xd8;
const uint8_t  WAKEUP = 0xab;
const uint8_t  DEEPSLEEP = 0xb9;
const uint8_t  PROGRAM = 0x02;
const uint8_t  GET_STATUS = 0x05;
const uint8_t  WRITE_ENABLE = 0x06;
const uint8_t  WRITE_DISABLE = 0x04;
const uint8_t  PROTECT = 0x36;
const uint8_t  UNPROTECT = 0x39;
const uint8_t  DEVICE_ID= 0x9f; 


//convenience
#define CS_ON  gpio_set_pin_level(SPI_FLASH_CS, false) 
#define CS_OFF gpio_set_pin_level(SPI_FLASH_CS, true) 

static struct io_descriptor * io = 0; 

void spi_flash_init()
{
  spi_m_sync_get_io_descriptor(&SPI_FLASH,&io); 
  spi_m_sync_enable(&SPI_FLASH); 
  CS_OFF; 
}

typedef struct status_register
{
 int bsy : 1; 
 int wel : 1; 
 int swp : 2;
 int wpp :1; 
 int epe: 1;
 int res: 1; 
 int sprl: 1; 
} status_register_t; 


static status_register_t _spi_get_status()
{
  status_register_t status; 
  CS_ON; 
  io_write(io,&GET_STATUS,1); 
  io_read(io, (uint8_t*) &status,1); 
  CS_OFF; 
  return status; 
}

int spi_flash_busy() 
{
  return _spi_get_status().bsy; 
}


void _spi_flash_write_enable()
{
  CS_ON; 
  io_write(io,&WRITE_ENABLE,1); 
  CS_OFF; 
}

void _spi_flash_write_disable()
{
  CS_ON; 
  io_write(io,&WRITE_DISABLE,1); 
  CS_OFF; 
}

static void _spi_flash_wait_until_ready() 
{
  status_register_t status = {0}; 
  CS_ON; 
  io_write(io,&GET_STATUS,1); 
  do { 
    int read = io_read(io,(uint8_t*) &status,1);
    if (!read)
    {
      io_write(io,&GET_STATUS,1); 
    }
  } while (status.bsy); 
  CS_OFF; 

}
void _spi_flash_change_protection(int protect, uint32_t start_addr, uint32_t end_addr) 
{
  uint32_t addr = start_addr; 
  if (end_addr == start_addr) end_addr+=1; 
  while (addr < end_addr) 
  {
   _spi_flash_wait_until_ready();
   _spi_flash_write_enable();
    uint8_t write_buf[4] = { protect ? PROTECT : UNPROTECT, (addr & 0x00ff0000) >> 16, (start_addr & 0x0000ff00) >> 8, (start_addr & 0xff)}; 
    CS_ON; 
    io_write(io, write_buf,4); 
    CS_OFF; 
    addr+=4096; 
  }
}




const uint32_t page_size = 256; 
const uint32_t block_size = 4096; 

static int _spi_flash_write(uint32_t addr, uint16_t len, const uint8_t * data) 
{

  //we need to divide our write into the page size
  uint32_t first_page = addr & 0xffffff00; 
  uint32_t last_page= (addr+len) & 0xffffff00; 
  int npage = 1 + ((last_page - first_page) >> 8); 
  int ipage = 0; 
  int written = 0;
  for (ipage = 0; ipage < npage; ipage++)
  {
    int offset = ipage==0 ? addr & 0xff : 0; 
    int size   = npage ==1 ? len : 
                 ipage == npage -1 ? addr+len - last_page 
                 : page_size - offset; 
    uint32_t start_addr = first_page + ipage*page_size + offset; 
    int data_offset = start_addr - addr; 
    uint8_t write_buf[4] = { PROGRAM, (start_addr & 0x00ff0000) >> 16, (start_addr & 0x0000ff00) >> 8, start_addr & 0xff}; 
    _spi_flash_wait_until_ready(); 
    _spi_flash_write_enable(); 
    CS_ON; 
    io_write(io,write_buf,4); 
    written+=io_write(io,data+data_offset, size); 
    CS_OFF; 
  }

  return written; 
}

static int _spi_flash_erase(uint32_t addr, int len)
{
  //we want to erase with the smallest number of calls possible. 
  // first figure out how many 4 kB blocks there are between addr + addr+len 

  int first_block = addr >> 12; 

  int last_block = (addr + len) >> 12; 
  //check if this is on a boundary or not. If it is, then last block can be a bit smaller
  if (((addr+len) & 0xfff) == 0) last_block--; 

  for (int block = first_block; block <= last_block; ) 
  {
    uint8_t write_buf[4] = {ERASE_4KB, block >> 4, (block & (0xf)) << 4 ,0} ; 

    //can we do a 64 kB flash? 
    if ( (block % 16) == 0 && block+16 <= last_block) 
    {
      write_buf[0] = ERASE_64KB; 
      block+=16; 
    }
    //if not, can we do a 32 kB flash? 
    else if ( (block % 8) == 0 && block+8 <= last_block) 
    {
      write_buf[0] = ERASE_32KB; 
      block+=8; 
    }
    else //just do a 4 kB flash
    {
      block++; 
    }

    _spi_flash_wait_until_ready(); 
    _spi_flash_write_enable();
    CS_ON;
    io_write(io,write_buf,4); 
    CS_OFF;
  }

  // TODO: figure out return value
  return 0; 
}

struct erase_context
{
  enum { ERASE_INP, ERASE_WAITING, ERASE_DONE} state;
  uint8_t first_block; 
  uint8_t last_block; 
  uint8_t iblock; 
}; 




void _init_erase_context(struct erase_context * ctx, int addr, int len ) 
{
  if (len == 0) 
  {
    //nothing to do 
    ctx->state=ERASE_DONE; 
    return; 
  }
  ctx->first_block = addr >> 12; 
  ctx->last_block = (addr + len) >> 12; 
  ctx->state = ERASE_WAITING; 
}

static int _spi_flash_erase_async( struct erase_context * ctx)
{
  //we want to erase with the smallest number of calls possible. 
  // first figure out how many 4 kB blocks there are between addr + addr+len 

  switch(ctx->state) 
  {
    case ERASE_WAITING:
    {
      status_register_t st= _spi_get_status(); 
      if (st.bsy) 
        break; 
      ctx->state = ERASE_INP; 
    }
      
    case ERASE_INP: 
      while (ctx->iblock <= ctx->last_block) 
      {
        int block = ctx->first_block+ctx->iblock; 
        uint8_t write_buf[4] = {ERASE_4KB, block >> 4, (block & (0xf)) << 4 ,0} ; 

        //can we do a 64 kB flash? 
        if ( (ctx->iblock % 16) == 0 && ctx->iblock+16 <= ctx->last_block) 
        {
          write_buf[0] = ERASE_64KB; 
          ctx->iblock+=16; 
        }
        //if not, can we do a 32 kB flash? 
        else if ( (ctx->iblock % 8) == 0 && ctx->iblock+8 <= ctx->last_block) 
        {
          write_buf[0] = ERASE_32KB; 
          ctx->iblock+=8; 
        }
        else //just do a 4 kB flash
        {
          ctx->iblock++; 
        }

        CS_ON;
        io_write(io,write_buf,4); 
        CS_OFF;
        ctx->state = ERASE_WAITING; 
        return 0; 
      }
      ctx->state = ERASE_DONE; 
    case ERASE_DONE:
      break; 
  }

  return 0; 
}


static int _spi_flash_read(uint32_t addr, uint16_t len, unsigned char * data) 
{
  uint8_t write_buf[5] = { READ_ARRAY, (addr & 0x00ff0000) >> 16 , (addr & 0x0000ff00) >> 8, addr & 0xff,0}; 

  int rd = 0;
  _spi_flash_wait_until_ready(); 
  //this needs to be tested to see if it actually works or if I need to do something smarter here to avoid a time gap! 
  CS_ON;
  io_write(io,write_buf,5); 
  rd = io_read(io,data,len); 
  CS_OFF; 
  return rd; 
}

int spi_flash_raw_read(uint32_t addr, int len, uint8_t * buf) 
{
  return _spi_flash_read(addr,len,buf); 
}


void spi_flash_deep_sleep()
{
  CS_ON; 
  io_write(io,&DEEPSLEEP,1); 
  CS_OFF; 
}

void spi_flash_wakeup()
{
  CS_ON; 
  io_write(io,&WAKEUP,1); 
  CS_OFF; 
  _spi_flash_wait_until_ready(); 
}


const uint32_t config_block_magic = (0xbcf9 << 16 ) | (CONFIG_BLOCK_BOOT_VERSION & 0xffff) ; 
const uint32_t config_block_app_magic = (0xacf9 << 16 ) | (CONFIG_BLOCK_APP_VERSION& 0xffff) ; 
const int n_config_slots = 16; //64 kB 


static int current_config_block = -1; 

static void _find_config_block() 
{
  uint32_t test; 
  int block; 
  for ( block = 0; block < n_config_slots; block++) 
  {
      _spi_flash_read(block_size * block, sizeof(test), (uint8_t*) &test); 
      if (test == config_block_magic)  // we found it! 
      {
        current_config_block = block; 
        break; 
      }
  }

}







static int spi_flash_read_config_block(config_block_t * config_block) 
{
  spi_flash_wakeup(); 
  // we have to find it 
  if (current_config_block < 0) 
  {
    _find_config_block(); 
  }


  // we couldn't find a valid config block anywhere, so default init! 
  if (current_config_block  < 0 ) 
  {
    default_init_block(config_block); 
    spi_flash_deep_sleep(); 
    return -1; 
  }

  //read in the app magic
  uint32_t app_test; 
  _spi_flash_read(current_config_block * block_size+sizeof(config_block_magic), sizeof(app_test), (uint8_t*) &app_test); 

  //don't read magic
  _spi_flash_read(current_config_block * block_size+sizeof(config_block_magic) + sizeof(config_block_app_magic), sizeof(config_block_t), (uint8_t*)  config_block); 


  //if app magic is not the same as magic, default init the application block
  if (app_test != config_block_app_magic) 
  {
    default_init_app_cfg(&config_block->app_cfg); 
    return -2; 
  }



  spi_flash_deep_sleep(); 
  return 0; 
}

static void spi_flash_write_config_block(const config_block_t * block) 
{
  spi_flash_wakeup(); 

  //try to find config block if not set
  if (current_config_block < 0) 
    _find_config_block(); 

  int next_config_block =( current_config_block + 1 ) % n_config_slots; 

  //be reasonably sure that the block is empty by reading the first 8 bytes 
  uint64_t check = 0; 
  _spi_flash_read(next_config_block * block_size, 8, (uint8_t*)  &check); 

  _spi_flash_change_protection(0, 0, block_size*n_config_slots); 
  if (check != 0xffffffffffffffff) 
  {
    //the last erase operation must have failed or something? 
    //  this kinda sucks, since this operation can take a while. 
    //  
    _spi_flash_erase(next_config_block * block_size, block_size); 
  }

  _spi_flash_write(next_config_block*block_size + sizeof(config_block_magic) + sizeof(config_block_app_magic), sizeof(config_block_t), (uint8_t*)  block); 

  uint32_t both_blocks[2] = { config_block_magic, config_block_app_magic };
  //write the magic AFTER the config block to make sure we wrote the config block! 
  _spi_flash_write(next_config_block * block_size, sizeof(both_blocks), (uint8_t *) both_blocks); 

  //erase the old config block, if there was one 
  //note that we could end up with two config blocks if we are turned off in the middle here, but that's ok  since 
  //our state is still stable. 
  if (current_config_block >=0) 
  {
    _spi_flash_erase(current_config_block * block_size, block_size); 
  }

  _spi_flash_change_protection(1, 0, block_size*n_config_slots); 
  current_config_block = next_config_block; 
  spi_flash_deep_sleep(); 
}

static int already_read_config_block = 0; 
static config_block_t the_config_block; 


config_block_t * config_block() 
{
  if (!already_read_config_block) 
  {
    spi_flash_read_config_block(&the_config_block); 
    already_read_config_block = 1; 
  }
  return &the_config_block;
}

void config_block_force_read() 
{
  spi_flash_read_config_block(&the_config_block); 
  already_read_config_block =1 ; 
}


void config_block_sync()
{
  spi_flash_write_config_block(&the_config_block); 
}



#define N_applications 4 
#define application_start  (64 * 1024)
#define application_size  (256-16)*1024
#define application_blocks  application_size/4096

static uint32_t application_offsets[4]; 

static uint32_t application_get_address(int slot, uint32_t offset) 
{
  return application_start + (slot-1) * application_size + offset; 
}

int spi_flash_application_erase_sync(int slot, int nblk) 
{

  if (nblk < 0 || nblk > application_blocks) nblk = application_blocks; 
  if (slot > N_applications || slot < 1) return -1; 
  uint32_t start_addr = application_get_address(slot,0); 
  uint32_t size = nblk << 12; 

   _spi_flash_change_protection(0, start_addr, start_addr+size);
   _spi_flash_erase(start_addr,size); 
   application_offsets[slot-1] = 0; //seek 
   return 0; 
}

static struct erase_context ectx[4] = {0};
static uint8_t erasing = 0; 

int spi_flash_application_erase_async(int slot, int nblk) 
{
  if (nblk < 0 || nblk > application_blocks) nblk = application_blocks; 
  if (slot > N_applications || slot < 1) return -1; 

  if (erasing & (1 < (slot-1)))
  {
    _spi_flash_erase_async(&ectx[slot-1]); 
    if ( ectx[slot-1].state == ERASE_DONE ) 
    {
      //clear the bit
      erasing &= ~(1 <<(slot-1));  
      application_offsets[slot-1] = 0; 
      return 0; 
    }
  }

  else
  {
    uint32_t start_addr = application_get_address(slot,0); 
    uint32_t size = nblk << 12; 
     _spi_flash_change_protection(0, start_addr, start_addr+size); 

    //set the erase bit 
    erasing |= ( 1 << (slot-1)); 
    _init_erase_context(&ectx[(slot-1)],start_addr, size); 
    _spi_flash_erase_async(&ectx[(slot-1)]); 
  }
  
  return 1+ectx[(slot-1)].last_block - ectx[slot].iblock; 
}



int spi_flash_application_seek(int slot, uint32_t offset) 
{
  if (slot > N_applications || slot < 1) return -1; 
  if (offset < application_size) 
  {
    application_offsets[slot-1] = offset; 
    return 0; 
  }
  return -1; 
}

int spi_flash_application_write(int slot, uint16_t len, const uint8_t * data) 
{

  if (slot > N_applications || slot < 1) return -1; 

  if (application_offsets[slot-1] + len > application_size)
  {
    len = application_size - application_offsets[slot-1]; 
  }

  uint32_t start_addr =application_get_address(slot, application_offsets[slot-1]);
  _spi_flash_change_protection(0, start_addr, start_addr+len);
  int total = 0; 
  while (total < len) 
  {
    int written = _spi_flash_write(start_addr+total, len-total, data+total); 
    total += written; 
  }
  _spi_flash_change_protection(1, start_addr, start_addr+len);
  application_offsets[slot-1] += total; 
  return total; 
}


int spi_flash_application_read(int slot, uint16_t len, uint8_t * data) 
{

  if (slot > N_applications || slot < 1) return -1; 

  if (application_offsets[slot-1] + len > application_size)
  {
    len = application_size - application_offsets[slot-1]; 
  }

  int rd = _spi_flash_read(application_get_address(slot, application_offsets[slot-1]), len, data); 
  if (rd > 0) 
  {
    application_offsets[slot-1] += rd; 
  }
  return rd; 
}


uint32_t spi_flash_device_id()
{
  CS_ON; 
  uint32_t val; 
  io_write(io,&DEVICE_ID,1); 
  io_read(io,(uint8_t*) &val,4); 
  CS_OFF;
  return val; 

}

