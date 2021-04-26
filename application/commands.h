#ifndef _rno_g_commands_h
#define _rno_g_commands_h



/** This parses a command and adds it to the queue if it checks out. */ 
int commands_put(uint8_t opcode, uint8_t payload_len, const uint8_t * cmd); 

/** This handles any ongoing things necessary for parsing of commands. Call it inthe main loop.  */ 
int commands_process(); 


#endif
