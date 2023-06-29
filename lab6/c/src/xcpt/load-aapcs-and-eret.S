#include "oscos/utils/save-ctx.S"

.section ".text"

load_aapcs_and_eret:
    load_aapcs
    eret

.type load_aapcs_and_eret, function
.size load_aapcs_and_eret, . - load_aapcs_and_eret
.global load_aapcs_and_eret
