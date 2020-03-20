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
    *(.fini*)
    *(.ctors*)
    *(.dtors*)
    . = ALIGN(4); 
    _efixed = .; 
  } > bootrom

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
