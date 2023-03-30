.globl from_el2_to_el1    // 宣告 from_el2_to_el1 作為 global symbol
from_el2_to_el1:
    mov x0, (1 << 31)    // 設定 x0 為 1 左移 31 位元後的值 (表示 EL1 使用 AArch64)
    msr hcr_el2, x0      // 將 x0 的值寫入 HCR_EL2 註冊 (Hypervisor Configuration Register)

    mov x0, 0x3c5        // 設定 x0 為 0x3c5 (EL1h=5，SPSel=1，順便設定中斷禁用)
    msr spsr_el2, x0     // 將 x0 的值寫入 SPSR_EL2 註冊 (Saved Program Status Register)

    msr elr_el2, lr      // 將返回地址 (LR) 的值寫入 ELR_EL2 註冊 (Exception Link Register)
    eret                 // 返回至 EL1，使用 ERET 指令 (Exception Return)