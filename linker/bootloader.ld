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
    *(.init*)
    *(.fini*)
    *(.ctors*)
    *(.dtors*)
    . = ALIGN(4); 
    _etext = .; 
    _efixed = .; 
  } > bootrom

  .data : 
  {
    . = ALIGN(4); 
    _srelocate  = .;
    _sdata  = .;
    *(.data*); 
    . = ALIGN(4); 
    _erelocate  = .;
    _edata  = .;
  } > ram AT > rom 

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
