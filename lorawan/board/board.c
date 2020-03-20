#include "board.h" 

uint32_t BoardGetRandomSeed(void) 
{

  return 4;

}

void BoardGetUniqueId( uint8_t *id )
{
  *id = 0x12; 

}



Version_t BoardGetVersion( void )
{
  Version_t v = {.Value = 0}; 
  return v; 
}
