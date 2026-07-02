.section .text
.globl _start
.type _start, @function

_start:
    # Load some data and return
    la a0, test_data
    ld a1, 0(a0)
    addi a0, a1, 1
    
    # Return
    jr ra

.section .data
test_data:
    .quad 0xDEADBEEFCAFEBABE

.section .bss
.bss_data:
    .quad 0
    .space 0x1000    # 4KB BSS area