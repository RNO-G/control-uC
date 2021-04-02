#ifndef _rno_g_debug_h
#define _rno_g_debug_h

#define DONT_STRIP  __attribute__((section (".keepme")))

//to avoid inlining! 
DONT_STRIP void dbg_gpio_set_pin_level(int pin, int level); 





#endif
