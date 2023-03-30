ENTRY(_start)   #從*(.text.relo)的_start開始執行
SECTIONS
{
  . = 0x60000;  #linker script定義的0x60000 不代表樹莓派會把這個程式自動load到此，是我們要自己從0x80000 relocate過來的
  _stext = .;   #同理user1.img的linker script 0x20000也是一樣，我們自己實作cpio_loader來load
  .text : {     #linker script比較像是告訴program變成process時該怎麼『由低到高位執行』，各區塊相對位子怎麼擺放，又依序會碰到誰這樣
    *(.text.relo) 
    _bl_entry = .;
    *(.text.boot)
    *(.text) 
    *(.rodata) 
  }
  . = ALIGN(0x1000);
  _etext = .;

  _sdata = .;
  .data : { *(.data) }
  . = ALIGN(0x1000);
  _edata = .;

  
  _sbss = .;
  .bss : { *(.bss*) } 
  . = ALIGN(0x1000);
  _ebss = .;  

  _blsize = _ebss - _stext;
}
#簡單來說，linker script 的功用就是在編譯鏡像檔案時，告訴編譯器和 linker 如何處理程式的各個部分，
#在運行時，loader 根據 linker script 的描述將鏡像檔案中的內容複製到內存中的對應位置，進而完成程式的加載和執行。
#因此，linker script 中的位置和屬性信息對應著實際的內存位址，是 loader 加載程序到內存中的依據。