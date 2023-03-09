__heap_size  = 0x8000;
__stack_size = 0x8000;
SECTIONS
{
    . = 0x80000;
    __start = .;
    .text : { 
        KEEP(*(.text.boot)) 
    }

    .rodata : { 
         *(.rodata .rodata.* .gnu.linkonce.r*) 
    }
    . = ALIGN(0x1000);

    PROVIDE(_data = .);

    .data : { 
        *(.data) 
    }
    . = ALIGN(0x1000);
    
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
    . = ALIGN(0x1000);

    .heap : {
		__heap_top = .;
		. = . + __heap_size;
		*(.heap)
	}
	. = ALIGN(0x1000);

 	.stack : {
		. = . + __stack_size;
		__stack_top = .;
 	}   
    __end = .;

 
}
__bss_size = (__bss_end - __bss_start)>>3;



