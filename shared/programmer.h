#ifndef _RNO_G_PROGRAMMER_H
#define _RNO_G_PROGRAMMER_H


/*Check if this is a programmer command. (This just searches for a #PRG prefix )
 * Returns 1 if it is, 0 if not */ 
int programmer_check_command(const char * cmd); 

/* Enter programmer mode, with the command, and the io device. 
 * Returns 0 if we entered succsefully, otherwise 1 (probably because we're already in programmer mode). 
 * This should only happen in the bootloader. 
 **/ 
struct io_descriptor; 
/** note that the programmer is allowed to overwrite parts of the command (so that we can decode base64 buffers in-place without copying)*/ 
int programmer_cmd(char * cmd, int len); 

/* Continues the programmer process, returns 0 if we are done with the programming, -1 if there was an error, or 1 if in progress. */ 
int programmer_process(); 


/* copy from application slot to flash 
 * can only be run by bootloader. 
 * Returns 0 when done, 1 to keep going, -1 if error. 
 * */
int programmer_copy_application_to_flash(int slot); 


/** Copy from flash to application (useful for debugging, maybe?) */ 
int programmer_copy_flash_to_application(int slot); 

#endif
