ENTRY(nomain)                                     
SECTIONS
{ 
  . = 0x08048000 + SIZEOF_HEADERS;

  /DISCARD/ : { *(.comment) }                   

  tinytext : { *(.text) *(.data) *(.rodata) }      
}
        
