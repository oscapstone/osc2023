.globl memzero
memzero:
	str xzr, [x20], #8   // 存放xzr暫存器的值0到x20所儲存的記憶體位置，然後x20加上8，即移動到下一個8個位元組的位置。
	subs x21, x21, #8   // 減去8個位元組的大小，以便計數器減少8個位元組。
	b.gt memzero        // 如果計數器仍大於0，則跳轉到符號memzero處繼續執行程式碼。
	ret                 // 返回程式呼叫者。