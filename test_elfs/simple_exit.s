.section .text
.globl _start
.type _start, @function

_start:
    li a7, 93
    li a0, 42
    ecall