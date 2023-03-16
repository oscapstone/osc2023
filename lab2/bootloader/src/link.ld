// 堆棧及堆大小
_heap_stack_size = 1M;

// 引導程序大小
_bootloader_size = 64K;

// 定義 SECTIONS 段
SECTIONS
{
    // 從地址 0x80000 開始
    . = 0x80000;
    _start = .;

    // 代碼段
    .text : { *(.text.boot) *(.text) }

    // 只讀數據段
    .rodata : { *(.rodata) }

    // 數據段
    .data : { *(.data) }

    // BSS 段
    .bss : {
        //* define a global symbol at bss top */
        _bss_top = .;// '.' 是指location counter，代表目前的記憶體位址
        *(.bss)
    }
    _bss_size = SIZEOF(.bss) >> 3;//bit transfer to bytes

    // 堆段
    .heap : {
        . = ALIGN(8);//ALIGN是對齊的意思，代表請對齊到最接近的8的倍數，相同或更大的記憶體位址
        _heap_top = .;//* define a global symbol at heap top */
    }

    // 堆棧大小為 _heap_stack_size
    . = . + _heap_stack_size;

    // 堆棧段
    .stack : {
        . = ALIGN(8);
        _stack_top = .;
    }

    // 程序結束
    _end = .;

    // 引導程序從地址 0x3000000 開始
    . = 0x3000000;
    _bootloader_relocated_addr = 0x3000000;

    // 引導程序大小為 _bootloader_size
    . = . + _bootloader_size;
}

// 計算代碼段的大小
__code_size = (_end - _start);
/**
_heap_stack_size：定義堆和堆棧的大小
_bootloader_size：定義引導程序的大小
SECTIONS：定義 ELF 鏈接脚本的 SECTIONS 部分
.text：代碼段，包括了.text.boot 和 .text
.rodata：只讀數據段
.data：數據段
.bss：BSS 段，用於存放未初始化的全局變量
_bss_size：BSS 段的大小
.heap：堆段，用於動態內存分配
.stack：堆棧段，用於函數調用和局部變量
_start：程序的入口點
_end：程序的結束點
_bootloader_relocated_addr：引導程序的重定位地址
__code_size：代碼段的大小
**/