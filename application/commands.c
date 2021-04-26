#include "include/rno-g-control.h" 
#include "application/commands.h" 
#include "application/report.h" 
#include "application/mode.h" 




int commands_process()
{
  ; //nothing to do here... for now 
  return 0; 
}



int commands_put(uint8_t  opcode, uint8_t payload_len, const uint8_t * payload) 
{

  switch (opcode) 
  {
    case RNO_G_CMD_SET_MODE: 
      if (payload_len != RNO_G_CMD_SET_MODE_SIZE) 
      {
        return -1; 
      }
      uint8_t mode = *payload; 
      if (mode >= RNO_G_NOT_A_MODE) 
      {
        //invalid 
        return -1; 
      }
      return mode_set((rno_g_mode_t) mode); 
    case RNO_G_CMD_REPORT: 

    default:
      return -1; 
  }

  return 0; 
}


