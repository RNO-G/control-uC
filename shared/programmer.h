#ifndef _RNO_G_PROGRAMMER_H
#define _RNO_G_PROGRAMMER_H




/*Check if this is a programmer command. 
 * Returns 1 if it is, 0 if not */ 
int programmer_check_command(const char * cmd); 

/* Enter programmer mode, with the command, and the io device. 
 * Returns 0 if we entered succsefully, otherwise 1 (probably because we're already in programmer mode). 
 **/ 
struct io_descriptor; 
int programmer_enter(const char * cmd, struct io_descriptor * io_descr); 

/* Continues the programmer process, returns 0 if we are done with the programming, -1 if there was an error, or 1 if in progress. */ 
int programmer_process(); 


/* copy from application slot to flash 
 * can only be run by bootloader
 * */
int programmer_copy_application_to_flash(int slot); 


#endif
