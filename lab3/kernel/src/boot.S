/* ARMv8 Assembly */

.section ".text.boot"

.global _start

_start:
    bl from_el2_to_el1

/* 16-18
設置例外向量表的地址。
首先，將 exception_vector_table 的address載入 x1 寄存器。
然後，將 x1 的值寫入 VBAR_EL1 register，以指示異常處理程序的基礎地址，這些異常處理程序在發生異常時會被執行。
VBAR_EL1 寄存器保存了任何發生異常時需要跳轉到的例外基地址，並且是在 EL1 中使用的。
*/
set_exception_vector_table:
    adr x1, exception_vector_table // vbar_el1: Vector Base Address Register (EL1)
    msr vbar_el1, x1               //           Holds the exception base address for any exception that is taken to EL1.

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

// 從 EL2 回到 EL1 的技巧是先設定 spsr_el2 與 elr_el2 並執行 eret。從 EL1 回到 EL0 也是相同技巧。
// SCTLR_EL2 (System Control Register (EL2)),ELR_EL2 (Exception Link Register (EL2)), eret :Exception Return.
/*
將程式從 EL2 切換到 EL1。
首先，設置 HCR_EL2 的 RW[31] bit為 1，以指示處理器將執行環境切換到 AArch64 的 EL1。
然後，將 SPSR_EL2 設置為 0x3c5，以指示保存當前狀態並將異常級別設置為 EL1h。
接著，將 ELR_EL2 設置為當前鏈接寄存器 lr 的值，以指示在執行返回操作時返回到該位置。
最後，使用 ERET 指令從異常處理程序返回到 EL1。
*/
from_el2_to_el1:
    mov x1, (1 << 31)              // hcr_el2: Execution state control for EL2
    msr hcr_el2, x1                //          RW[31]: 0b1 The processor execution environment for EL1 is AArch64
    mov x1, 0x3c5                  // spsr_el2: Holds the saved process state when an exception is taken to EL2.
    msr spsr_el2, x1               //           (1)[1111] 00 (2)[0101] -> (1) EL2-PSTATE.DAIF Disabled (2) Exception level = EL1h
    msr elr_el2, lr                // elr_el2: When taking an exception to EL2, holds the address to return to.
    eret                           // eret: Perform an exception return. EL2 -> EL1

//mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled  ,el2 interrupt disable