.section ".text.boot"

.global _start

_start:
	bl from_el2_to_el1

set_exception_vector_table:
	adr x1, exception_vector_table  //將exception vector table的位址寫到vbar_el1
	msr vbar_el1, x1

setup_stack:
    ldr     x1, =_stack_top
    mov     sp, x1                 // sp = x1

setup_bss:
    ldr     x1, =_bss_top          // load 32bits to x1 from _bss_top
    ldr     w2, =_bss_size         // load 64bits to w2 from _bss_size

init_bss:
    cbz     w2, run_main
    str     xzr, [x1], #8          // store xzr to addr[x1], then x1=x1+8
    sub     w2, w2, #1
    cbnz    w2, init_bss           // branch if w2 != 0

run_main:
    bl      main                   // branch and update lr with "main"

proc_hang:
    wfe                            // waiting in low-power state
    b       proc_hang

from_el2_to_el1:
	mov x1, (1<<31)		//hcr_el2: hypervisor configuration register
	msr hcr_el2, x1		
	mov x1, 0x3c5		//spsr_el2: save process state register
	msr spsr_el2, x1	
	msr elr_el2, lr 	//elr_el2: return address ; lr: link register
	eret				//exception return (el2->el1)



/*hcr_el2 設bit[31]為1 -> Execution state control for lower Exception levels(1:Aarch64; 0: Aarch32)
spsr_el2 設 0x3c5-> 0b00111100101; 1111:DAIF disable interrupt; 0101:Exception level=EL1h
*/