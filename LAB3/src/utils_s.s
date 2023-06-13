.global branchAddr // 宣告branchAddr這個符號為全域性質，可以在其他檔案中被使用。
branchAddr: // 定義符號branchAddr
   br x0 // 跳轉到暫存器x0所存放的地址處執行指令。

.globl get_el // 宣告get_el這個符號為全域性質，可以在其他檔案中被使用。
get_el: // 定義符號get_el
   mrs x0, CurrentEl // 將目前的異常等級讀取到暫存器x0中。
   lsr x0, x0, #2 // 將x0暫存器中的值向右移動2位，相當於除以4，以獲取異常等級的值。
   ret // 返回到呼叫這個函式的地方。