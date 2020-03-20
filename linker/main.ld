OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

INCLUDE map.ld 
INCLUDE stack.ld


/* Section Definitions */
SECTIONS
{

  .text :
  {
    . = ALIGN(4); 
    _stext = .; 
    _sfixed = .; 
    KEEP(*(.vectors* .vectors.*)) 
    *(.text*) 
    *(.rodata*) 
    . = ALIGN(4);
    KEEP(*(.init))
    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
    __preinit_array_end = .;

    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;

    . = ALIGN(4);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))

    . = ALIGN(4);
    KEEP(*(.fini))
    . = ALIGN(4);
    __fini_array_start = .;
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    __fini_array_end = .;

    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

 
    . = ALIGN(4); 
    _efixed = .; 
  } > rom

  /* .ARM.exidx is sorted, so has to go in its own output section.  */
  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx :
  {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > rom
  PROVIDE_HIDDEN (__exidx_end = .);

  . = ALIGN(4); 
  _etext = .; 

  .relocate : AT (_etext)  
  {
    . = ALIGN(4); 
    _srelocate  = .;
    _sdata  = .;
    *(.ramfunc .ramfunc.*);
    *(.data*); 
    . = ALIGN(4); 
    _erelocate  = .;
    _edata  = .;
  } > ram 

  .bss (NOLOAD) :
  {
    . = ALIGN(4); 
    _sbss = .; 
    _szero = .; 
    *(.bss*) 
    *(COMMON) 
    . = ALIGN(4); 
    _ezero = .; 
    _ebss = .; 
  } > ram

  .stack (NOLOAD) :
  {
    . = ALIGN(8) ; 
    _sstack = .; 
    . = . + STACK_SIZE; 
    . = ALIGN(8); 
    _estack = .; 
  } > ram 

  . = ALIGN(4); 
  _end = .; 
}
