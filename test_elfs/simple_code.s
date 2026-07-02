.section .text
.globl _start
.type _start, @function

_start:
    # Simple infinite loop with a marker instruction
    # This will be our test code
    addi x0, x0, 0x13    # marker: 0x9308d093
    
    # Infinite loop
    j _start

.size _start, .-_start