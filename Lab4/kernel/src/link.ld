

SECTIONS
{
    . = 0x80000;
    __start = .;

    .text : { 
        KEEP(*(.text.boot)) 
        . = ALIGN(0x1000);
        }

    .rodata : { 
        *(.rodata .rodata.* .gnu.linkonce.r*) 
        . = ALIGN(0x1000);
        }

    .data : { 
        *(.data) 
        . = ALIGN(0x1000);
        }
    

    .bss (NOLOAD) : {
        . = ALIGN(0x1000);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
   

    .heap : {
        . = ALIGN(8);
		__heap_top = .;

		*(.heap)
	}
	. = . + __heap_size;

 	.stack : {
        . = ALIGN(8);
		__stack_top = .;
        *(.stack)
        __stack_end = .;
 	}   
    __end = .;
}

__bss_size = (__bss_end - __bss_start)>>3;
__heap_size  = 0x1000;
__stack_size = 0x1000;


