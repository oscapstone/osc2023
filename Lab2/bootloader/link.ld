

SECTIONS
{
    . = 0x60000;
    __bootstart = .;
    . = 0x80000;
    __start = .;
    .text : { 
        KEEP(*(.text.boot)) 

        }

    .rodata : { 
        *(.rodata .rodata.* .gnu.linkonce.r*) 

        }

    .data : { 
        *(.data) 

        }

    .bss : {

        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
   

    .heap : {
		__heap_top = .;
		. = . + __heap_size;
		*(.heap)
	}
	

 	.stack : {
		__stack_top = .;
        *(.stack)
        __stack_end = .;
 	}   
    __end = .;

}
__bss_size = (__bss_end - __bss_start )>>3;
__heap_size  = 0x1000;
__stack_size = 0x1000;


__code_size = (__end - __start);