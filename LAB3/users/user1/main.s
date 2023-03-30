.section ".text"
.global _start
_start:
    mov x0, 0
1:
    add x0, x0, 1
    svc 0   //填寫 0 表示不傳遞任何參數，只是單純地引發一個例外並進入 Supervisor 模式，讓後續的程式能夠執行一些特權操作。
    cmp x0, 5
    blt 1b
1:      //無窮迴圈
    b 1b
