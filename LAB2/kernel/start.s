.section ".text.kernel"  //指定目前的程式段為.text.kernel

.globl _start  // 告知這個label可以被外部程式使用

_start:  // 開始執行這個label
    ldr    x1, =_dtb_ptr  // 將_dtb_ptr的位置儲存到x1暫存器中
    str    x0, [x1]   // 將x0暫存器的值（dtb's address）存到_dtb_ptr所在位置中
    mrs    x20, mpidr_el1        // 讀取主處理器ID（Multiprocessor Affinity Register）
    and    x20, x20,#0xFF        // 以位元運算& 0xFF檢查處理器ID是否為0
    cbz    x20, master        // 如果處理器ID不是0，就跳轉到master label

hang:   // 若處理器ID不是0就一直卡住
    b hang

master:  // 處理器ID為0時的label
    adr    x20, _sbss   // 將_sbss位置（即bss段的起始位置）寫入x20
    adr    x21, _ebss   // 將_ebss位置（即bss段的結束位置）寫入x21
    sub    x21, x21, x20  // 計算bss段的大小
    bl     memzero   // 使用memzero函數來清空bss段

    mov    sp, #0x400000  // 設定堆疊指標，從記憶體地址0x400000開始向下生長
    bl    kernel_main    // 呼叫kernel_main函數

.global _dtb_ptr  // 告知這個label可以被外部程式使用

.section .data  // 指定目前的程式段為.data
_dtb_ptr: .dc.a 0x0  // 在data段中，設定_dtb_ptr變數初始值為0
