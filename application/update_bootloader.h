#ifndef _rno_g_update_bootloader
#define _rno_g_update_bootloader


//checks if bootloader is up to date with main image
int bootloader_check();
int bootloader_update();
int bootloader_is_locked();
int bootloader_lock();
int bootloader_unlock();

#endif
