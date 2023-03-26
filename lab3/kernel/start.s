.section ".text.boot"

.global _start

_start:
    mrs    x0, mpidr_el1                /* move the contents of a special register to a general-purpose register */
    and    x0, x0, #3                   /* read [0:2] and get CPU ID */
    cbz    x0, _core                    /* go to _core if CPU ID equals to 0 */

_halt: 
    wfe                                 /* wait for event */
    b      _halt                        /* halt the core */

_core:
    bl     _from_el2_to_el1             /* switch to EL1 */
    bl     _set_exception_vector_table  /* set up exception vector table */
    ldr    x0, =_start                  /* set stack top to the head of text section */
    mov    sp, x0
    ldr    x0, =__bss_start             /* initialize the variables in bss section to 0 */
    ldr    x1, =__bss_end
    sub    x1, x1, x0

_zero:
    cbz    x1, _kernel                  /* go to _kernel if bss section is 0 */
    str    xzr, [x0], #8
    sub    x1, x1, #8
    cbnz   x1, _zero

_kernel:
    bl     main
    b      _halt                       /* halt the core if it returns */
