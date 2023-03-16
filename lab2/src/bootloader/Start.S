#include "mm.h"

.section ".text.boot"

_start:
    adr     x0, _start      //load start address to register x0
    ldr     x1, =0x60000    //load immediate value 0x60000 to x1 ; for bootloader kernel
    mov     x2, x1          //copy x1 to x2
    adr     x4, _end
    cmp     x0, x1          //if x0 == x1, branch to after_moved ; if not, do self relocation
    beq     after_moved
relocation:          
    //self relocation
    cmp     x4, x0
    bls     to_target
    ldr     x3, [x0], #8     /* 將(x0+0)地址中的內容值放入x3,並且 --> x0=x0+8 ; Def: memory--->register 將記憶體的內容值LOAD進通用暫存器R0~R12*/
    str     x3, [x1], #8     /* x3的內容值放入x1的位址,並且 --> x1=x1+8 ; register--->memory 將src暫存器的內容直搬到搬到一記憶體位址 */
    b       relocation

to_target:
    br x2

after_moved:
    // Clear bss
    adr     x0, bss_begin
    adr     x1, bss_end
    sub     x1, x1, x0
    bl      memzero         //jump to function 'memzero' in mm.S to set zero to [x0], #8 

    // Jump to C code
    mov     sp, #LOW_MEMORY
    bl      boot_main       //jump to main func of bootloader

end:
    wfe
    b end


/*
bootloader一進去會先確認PC跟我期望的位置合不合
在這作業，PC（板子的機制）是0x80000，我們期望的位置是0x60000
bootloader發現address不一樣，就會手動set 0x60000把kernel 的東西搬過去
 */