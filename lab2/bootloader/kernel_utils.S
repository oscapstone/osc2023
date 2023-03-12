.section ".text"

.globl store_kernel_image
store_kernel_image:
    str     x0, [x1]
    ret

.global jump_to
jump_to:
    br      x0
