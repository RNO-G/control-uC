/** Linker memory map, shared between application and bootloader */ 

MEMORY
{

  /* 8 kB for the bootloader */ 
  bootrom  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x0004000

  /* The rest for the application */ 
  rom (rx)        : ORIGIN = 0x00004000, LENGTH = 0x003c000 

  /* 256 bytes of shared memory between bootloader and application  (for reset counting and error reporting)*/ 
  ram       (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00007f00 
  shared    (rwx) : ORIGIN = 0x20007f00, LENGTH = 0x0000100 
}


__boot_rom_start__ = ORIGIN(bootrom); 
__boot_rom_size__ = LENGTH(bootrom); 

__rom_start__ = ORIGIN(rom); 
__rom_size__ = LENGTH(rom); 

__shared_start__ = ORIGIN(shared); 
__shared_size__ = LENGTH(shared); 

__ram_start__ = ORIGIN(ram); 
__ram_size__ = LENGTH(ram); 


