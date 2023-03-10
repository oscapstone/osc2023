.section ".text.boot"

.global _start

_start:
    mrs    x0, mpidr_el1     /* move the contents of a special register to a general-purpose register */
    and    x0, x0, #3        /* read [0:2] and get CPU ID */
    cbz    x0, _core         /* go to _core if CPU ID equals to 0 */

_halt: 
    wfe                      /* wait for event */
    b     _halt              /* halt the core */

_core:
    adr    x0, __bss_start
    adr    x1, __bss_size

_zero:
    cbz    x1, _relocate     /* go to _relocate if bss section is 0 */
    str    xzr, [x0], #8     /* initialize the variables in bss section to 0 */
    sub    x1, x1, #8
    cbnz   x1, _zero

_relocate:
    ldr   x0, =__begin
    ldr   x1, =__end
    sub   x1, x1, x0         /* calculate the block size needed to be relocated*/
    ldr   x2, =__relocation

_loop:
    ldr   x3, [x0], #8
    str   x3, [x2], #8       /* relocate the block */
    sub   x1, x1, #8
    cbnz  x1, _loop

_done:
    ldr   x0, =bootloader
    ldr   x1, =__begin
    ldr   x2, =__relocation  
    sub   x0, x0, x1
    add   x0, x0, x2         /* relocate bootloader */
    mov   sp, #0x80000       /* move stack pointer to 0x80000 */
    br    x0                 /* branch to relocated bootloader */
