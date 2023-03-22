.section ".text"

.macro save_all						// save general registers to stack
    sub sp, sp, 32 * 8				//32*8 = 16*16
    stp x0, x1, [sp ,16 * 0]		//store pair of registers to sp+16*0
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm								//end macro

.macro load_all						// load general registers from stack
    ldp x0, x1, [sp ,16 * 0]		//load pair of registers from sp+16*0
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

exception_handler:
    save_all
    bl	 	exception_entry
	mov		x2,0x0
	msr		SPSR_EL1,x2
	bl		core_timer_enable	
    load_all
    eret

/******************vector table********************/
.align 11							//0x800 : 2^11
.global exception_vector_table
exception_vector_table:
	b exception_handler				// branch to a handler function.
	.align 7 						// entry size is 0x80 : 2^7
	b exception_handler
	.align 7
	b exception_handler
	.align 7
	b exception_handler
	.align 7

	b exception_handler
	.align 7
	b exception_handler
	.align 7
	b exception_handler
	.align 7
	b exception_handler
	.align 7

	b exception_handler
	.align 7
	b core_timer_handler
	.align 7
	b exception_handler
	.align 7
	b exception_handler
	.align 7

	b exception_handler
	.align 7
	b exception_handler
	.align 7
	b exception_handler
	.align 7
	b exception_handler
	.align 7

.global set_exception_vector_table		//use as function
set_exception_vector_table:
	adr x0,exception_vector_table		//adr : load address to a register
	msr vbar_el1,x0						//vbar_el1 : Vector Base Address Register (EL1)
	ret

core_timer_enable:
    mov     x0,1
    msr     CNTP_CTL_EL0,x0     //bit0 : timer enabled , bit1 : IMASK(1: won't trigger interrupt)
    mrs     x0,CNTFRQ_EL0       //frequency of the system
    msr     CNTP_TVAL_EL0,x0    //on a write of this register , CNTP_CVAL_EL0 is set to (CNTPCT_EL0 + timer value)
                                //timer value for thr EL1 physical timer , if(CNTP_CTL_EL0.enable is 1)->timer met when (CNTPCT_EL0-CNTP_CVAL_EL0) >= 0
    ldr     x1,=0x40000040		//core0 timers interrupt control address : 0x40000040
   	mov     x0,2   				//bit1 : IRQ enable
    str     w0,[x1]             //unmask timer interrupt
	ret

core_timer_handler:
    save_all
	mrs     x0,CNTFRQ_EL0
	mov		x1,2
	mul		x0,x0,x1
    msr     CNTP_TVAL_EL0,x0
    bl      print_time
	load_all
    eret
