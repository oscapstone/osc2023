.section ".text._start"

_start:
    // We don't need to clear the .bss section here, since the kernel does this
    // for us.

    // Call the main function.
    bl main

    // Terminate the program.
    b exit

.size _start, . - _start
.type _start, function
.global _start
