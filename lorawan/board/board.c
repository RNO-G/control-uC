#include "board.h" 
#include "shared/spi_flash.h" 


uint32_t BoardGetRandomSeed(void) 
{
  return 4; 

  /*
  union 
  {
    uint16_t id16[2]; 
    uint32_t id32; 
  } id; 
  BoardGetUniqueId(&id.id16[0]);
  id.id16[1] = id.id16[0]; 
  return SysTick->VAL ^ id.id32 ;
  */

}
static int iid = -1; 

void BoardGetUniqueId( uint16_t *id )
{
  if (iid < 0) 
  {
    iid = config_block()->app_cfg.station_number;  
  }
  *id = (uint16_t) iid; 

}



Version_t BoardGetVersion( void )
{
  Version_t v = {.Value = 0}; 
  return v; 
}
