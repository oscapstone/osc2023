.section ".text.boot"   // Define a section named ".text.boot", typically for boot code.

.global _start          // Make the label '_start' globally visible, entry point for the linker.

_start:                 // The entry point of the code.
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1   // Move the value of MPIDR_EL1 (Multiprocessor Affinity Register)
                            // into x1. This register holds the CPU's unique ID.
    and     x1, x1, #3      // Perform an AND operation on x1 with 3 (0b11) to isolate
                            // the lowest two bits of the MPIDR. These bits represent
                            // the CPU core ID within a CPU cluster.

    cbz     x1, setting     // Compare x1 to zero. If it is zero (meaning this is core 0),
                            // continue to 'setting'. Otherwise, proceed to the next instruction.

    // cpu id > 0, stop
halt:   wfe                 // Wait for Event. Puts the processor in a low-power state until
                            // an event or interrupt occurs.
        b       halt        // Unconditional branch to 'halt', creating an infinite loop
                            // that effectively stops the CPU core.

setting: ldr     x1, =_start    // Load the address of '_start' into x1. This is typically
                                // used to set up the stack pointer.

         mov     sp, x1         // Move the value of x1 into the stack pointer register (sp).
                                // This sets up the stack at the start of the boot section.

         ldr     x1, =__bss_start   // Load the address of the start of the BSS section
                                    // into x1. BSS is used for zero-initialized variables.

         ldr     w2, =__bss_size    // Load the size of the BSS section into w2.

clear_bss: cbz    w2, kernel_main  // Compare w2 to zero. If zero, all of BSS is cleared,
                                   // jump to 'kernel_main'. Otherwise, continue to clear BSS.

           str    xzr,[x1],#8      // Store the value in xzr (zero register) at the address in x1,
                                   // then increment x1 by 8 bytes. This clears 8 bytes of BSS.

           sub    w2, w2, #1       // Subtract 1 from w2. Since w2 holds the size of the BSS
                                   // in 8-byte units, this decrements the counter.

           cbnz   w2, clear_bss    // Compare w2 with zero. If not zero, more of BSS needs clearing,
                                   // so branch back to 'clear_bss'.

kernel_main: bl    main             // Branch with link to 'main'. This is where the main
                                    // application or kernel initialization code begins.
                                    // 'bl' stores the return address in the link register (lr).

     b       halt                   // Unconditional branch to 'halt'. If 'main' returns,
                                    // stop the CPU core by entering the halt state.
