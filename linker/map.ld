/** Linker memory map, shared between application and bootloader */ 

MEMORY
{

  /* 8 kB for the bootloader */ 
  bootrom  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x0004000

  /* The rest for the application */ 
  rom (rx)        : ORIGIN = 0x00004000, LENGTH = 0x003c000 

  /* 256 bytes of shared memory between bootloader and application  (for reset counting and error reporting)*/ 
  shared    (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00000100 
  ram       (rwx) : ORIGIN = 0x20000100, LENGTH = 0x00007f00 
}


__boot_rom_start = ORIGIN(bootrom); 
__boot_rom_size = LENGTH(bootrom); 

__rom_start = ORIGIN(rom); 
__rom_size = LENGTH(rom); 

__shared_start = ORIGIN(shared); 
__shared_size = LENGTH(shared); 

__ram_start = ORIGIN(ram); 
__ram_size = LENGTH(ram); 


