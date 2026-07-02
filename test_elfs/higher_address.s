.section .text
.globl _start
.type _start, @function

_start:
    # Code to test at different address
    li t0, 0x12345678
    li t1, 0x87654321
    add t2, t0, t1
    
    jr ra

.size _start, .-_start