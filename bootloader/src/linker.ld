SECTIONS
{
    .=0x80000;
    .bootloader :
    {
        KEEP(*(.text.bootloader))
    }
    . = ALIGN(4096);
    _begin = .;

    
    



    _end = .;
}