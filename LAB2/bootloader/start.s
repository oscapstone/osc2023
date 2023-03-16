.section ".text.relo"
.globl _start
// 這段程式碼實現的是 U-Boot 的啟動過程，首先需要將 U-Boot 自身的韌體從地址 0x80000 轉移到 0x60000。
// 在這個過程中，程序讀取 0x80000~0x80000+_blsize 的韌體數據，然後寫入 0x60000 開始的地址。
// 接下來的程序會清空 .bss 段，設置堆棧指針，然後呼叫 bootloader_main 函數，開始進入 U-Boot 的主流程。
// 在這個過程中，程序還會獲取 CPU 的 ID，判斷是否為主核心。如果不是主核心，程序就會一直停留在 hang 標籤

// 重新定位引導程序，將其從 0x80000 移動到 0x60000
_start:
    adr x10, .          // x10=0x80000，將當前位置的地址寫入 x10
    ldr x11, =_blsize   // 將 _blsize 變數的地址載入 x11
    add x11, x11, x10   // 計算出 _blsize 在新地址下的位置，寫入 x11
    ldr x12, =_stext    // x12=0x60000，將 _stext 變數的地址寫入 x12

moving_relo:
    cmp x10, x11        // 比較 x10 和 x11 的值，檢查是否已經處理完整個引導程序
    b.eq end_relo       // 如果相等，則跳轉到 end_relo 標籤
    ldr x13, [x10]      // 從 x10 指向的位置讀取 8 個字節的數據，存入 x13 中
    str x13, [x12]      // 將 x13 中的數據寫入 x12 指向的位置，完成數據的移動
    add x12, x12, #8    // 將 x12 加上 8，以指向下一個位置
    add x10, x10, #8    // 將 x10 加上 8，以指向下一個位置
    b moving_relo       // 跳轉到 moving_relo 標籤，繼續處理下一個字節

end_relo:
    ldr x14, =_bl_entry    // 載入 _bl_entry 變數的地址，存入 x14 中
    br x14               // 跳轉到 x14 中存儲的地址，即引導程序的入口地址


.section ".text.boot"
.globl _start_bl
    ldr x20, =_dtb     // 將 _dtb 變數的地址載入 x20
    str x0, [x20]      // 將 x0 中的值寫入 x20 指向的位置，即 _dtb 變數
    mrs x20, mpidr_el1        
    and x20, x20,#0xFF // 檢查 CPU 的處理器 ID
    cbz x20, master   // 如果是主 CPU，則跳轉到 master 標籤，否則進入死循環

hang:
    b hang              // 進入死循環

master:
    // 清空 .bss 段
    adr x20, _sbss  // 將 _sbss 變數的地址寫入 x20
    adr x21, _ebss  // 將 _ebss 變數的地址寫入 x21
    sub x21, x21, x20  
    bl  memzero

    mov sp, #0x400000    // 4MB設置堆棧指針
    bl  bootloader_main  // 呼叫 bootloader_main 函數
    
.global _dtb    // 定義 device tree 的指針

.section .data
_dtb: .dc.a 0x0
