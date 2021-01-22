#ifndef _rno_g_commands_h
#define _rno_g_commands_h


typedef enum rno_g_msg_type
{
  MSG_REPORT_REQUEST 


} rno_g_msg_type_t; 


typedef struct rno_g_msg
{
  rno_g_msg_type_t msg_type; 
  rno_g_msg_flags_t flags; 
  uint8_t msg_len; 
  uint8_t msg_payload[];  // of length msg_len 
} rno_g_msg_type; 





enum 
{
  COMMAND_SEND_REPORT, // report status asap
  COMMAND_SET_POWER_STATE, // change power state 
  COMMAND_SET_SBC_BOOT_SELECTION,  // change SBC boot selection (0 for emmc, 1 for sd card) 
  COMMAND_SEND_SBC_COMMAND
}




#endif
