.section ".text.kernel"    // 宣告 section 為 ".text.kernel"
.globl _start             // 宣告 _start 為 global symbol

_start:
    /*          看來 *_dtb_ptr = x0 = *_dtb = dtb的loading address        */
    ldr    x1, =_dtb_ptr   // 將 _dtb_ptr 的位置載入 x1
    str    x0, [x1]        // 將 x0(即dtb樹狀結構的地址) 寫入 _dtb_ptr 所指向的記憶體位置

    /*          cpu id pass         */
    mrs    x20, mpidr_el1   // 讀取 MPIDR_EL1 註冊到 x20
    and    x20, x20,#0xFF  // 將 x20 與 0xFF 做 bitwise AND 以檢查處理器 ID
    cbz    x20, master     // 如果 x20 為零，則跳轉至 master 標籤，否則繼續執行 hang

hang:
    b hang                  // 無限迴圈等待

master:
    bl     from_el2_to_el1  // 呼叫 from_el2_to_el1 子程式以切換至 EL1, 下一個指令在 EL1 中運行，而 bl 會將該地址儲存至slnker reg

    /* setup interrupt vector base 讓使用者能藉由exception跳會底層EL */
    ldr x0, =el1_vector_base   // 載入 el1_vector_base 的位置到 x0
    msr vbar_el1, x0           // 將 x0 的值寫入 VBAR_EL1 註冊以設置中斷向量表,用於設置當EL1正在運行時的中斷向量表的基地址
    

    adr    x20, _sbss   // 載入 _sbss 的位置到 x20
    adr    x21, _ebss   // 載入 _ebss 的位置到 x21
    sub    x21, x21, x20 // 計算出 BSS 區域的大小
    bl     memzero      // 呼叫 memzero 子程式以將 BSS 區域初始化為 0

    mov    sp, #0x200000  // 將堆疊指標設置在 2MB 的位置
    bl     kernel_main    // 呼叫 kernel_main 子程式以開始執行核心代碼

.global _dtb_ptr   // 宣告 _dtb_ptr 為 global symbol
.section .data     // 宣告 section 為 ".data"
_dtb_ptr: .dc.a 0x0  // 初始化 _dtb_ptr 為 0
