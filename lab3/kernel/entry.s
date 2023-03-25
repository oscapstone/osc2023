.section ".text"

.global from_el2_to_el1

from_el2_to_el1:
    mov    x0, (1 << 31)  /* EL1 uses aarch64 */
    msr    hcr_el2, x0
    mov    x0, 0x3c5      /* EL1h (SPSel = 1) with interrupt disabled */
    msr    spsr_el2, x0   /* save the current processor's state in spsr_el2 */
    msr    elr_el2, lr    /* save the exception return address in elr_el2 */
    eret                  /* return to EL1 */
