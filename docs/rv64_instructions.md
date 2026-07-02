# lui

load upper immediate.

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="5">imm[31:12]</td>
<td>rd</td>
<td>01101</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
lui rd,imm

Description  
Build 32-bit constants and uses the U-type format. LUI places the
U-immediate value in the top 20 bits of the destination register rd,
filling in the lowest 12 bits with zeros.

Implementation  
x\[rd\] = sext(immediate\[31:12\] &lt;&lt; 12)

# auipc

add upper immediate to pc

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="5">imm[31:12]</td>
<td>rd</td>
<td>00101</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
auipc rd,imm

Description  
Build pc-relative addresses and uses the U-type format. AUIPC forms a
32-bit offset from the 20-bit U-immediate, filling in the lowest 12 bits
with zeros, adds this offset to the pc, then places the result in
register rd.

Implementation  
x\[rd\] = pc + sext(immediate\[31:12\] &lt;&lt; 12)

# addi

add immediate

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">imm[11:0]</td>
<td>rs1</td>
<td>000</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
addi rd,rs1,imm

Description  
Adds the sign-extended 12-bit immediate to register rs1. Arithmetic
overflow is ignored and the result is simply the low XLEN bits of the
result. ADDI rd, rs1, 0 is used to implement the MV rd, rs1 assembler
pseudo-instruction.

Implementation  
x\[rd\] = x\[rs1\] + sext(immediate)

# slti

set less than immediate

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">imm[11:0]</td>
<td>rs1</td>
<td>010</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
slti rd,rs1,imm

Description  
Place the value 1 in register rd if register rs1 is less than the
signextended immediate when both are treated as signed numbers, else 0
is written to rd.

Implementation  
x\[rd\] = x\[rs1\] &lt;s sext(immediate)

# sltiu

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">imm[11:0]</td>
<td>rs1</td>
<td>011</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sltiu rd,rs1,imm

Description  
Place the value 1 in register rd if register rs1 is less than the
immediate when both are treated as unsigned numbers, else 0 is written
to rd.

Implementation  
x\[rd\] = x\[rs1\] &lt;u sext(immediate)

# xori

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">imm[11:0]</td>
<td>rs1</td>
<td>100</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
xori rd,rs1,imm

Description  
Performs bitwise XOR on register rs1 and the sign-extended 12-bit
immediate and place the result in rd  
Note, "XORI rd, rs1, -1" performs a bitwise logical inversion of
register rs1(assembler pseudo-instruction NOT rd, rs)

Implementation  
x\[rd\] = x\[rs1\] ^ sext(immediate)

# ori

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">imm[11:0]</td>
<td>rs1</td>
<td>110</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
ori rd,rs1,imm

Description  
Performs bitwise OR on register rs1 and the sign-extended 12-bit
immediate and place the result in rd

Implementation  
x\[rd\] = x\[rs1\] | sext(immediate)

# andi

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">imm[11:0]</td>
<td>rs1</td>
<td>111</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
andi rd,rs1,imm

Description  
Performs bitwise AND on register rs1 and the sign-extended 12-bit
immediate and place the result in rd

Implementation  
x\[rd\] = x\[rs1\] & sext(immediate)

# slli

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>0X</td>
<td>shamt</td>
<td>rs1</td>
<td>001</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
slli rd,rs1,shamt

Description  
Performs logical left shift on the value in register rs1 by the shift
amount held in the lower 5 bits of the immediate  
In RV64, bit-25 is used to shamt\[5\].

Implementation  
x\[rd\] = x\[rs1\] &lt;&lt; shamt

# srli

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>0X</td>
<td>shamt</td>
<td>rs1</td>
<td>101</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
srli rd,rs1,shamt

Description  
Performs logical right shift on the value in register rs1 by the shift
amount held in the lower 5 bits of the immediate  
In RV64, bit-25 is used to shamt\[5\].

Implementation  
x\[rd\] = x\[rs1\] &gt;&gt;u shamt

# srai

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>01000</td>
<td>0X</td>
<td>shamt</td>
<td>rs1</td>
<td>101</td>
<td>rd</td>
<td>00100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
srai rd,rs1,shamt

Description  
Performs arithmetic right shift on the value in register rs1 by the
shift amount held in the lower 5 bits of the immediate  
In RV64, bit-25 is used to shamt\[5\].

Implementation  
x\[rd\] = x\[rs1\] &gt;&gt;s shamt

# add

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>000</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
add rd,rs1,rs2

Description  
Adds the registers rs1 and rs2 and stores the result in rd.  
Arithmetic overflow is ignored and the result is simply the low XLEN
bits of the result.

Implementation  
x\[rd\] = x\[rs1\] + x\[rs2\]

# sub

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>01000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>000</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sub rd,rs1,rs2

Description  
Subs the register rs2 from rs1 and stores the result in rd.  
Arithmetic overflow is ignored and the result is simply the low XLEN
bits of the result.

Implementation  
x\[rd\] = x\[rs1\] - x\[rs2\]

# sll

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>001</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sll rd,rs1,rs2

Description  
Performs logical left shift on the value in register rs1 by the shift
amount held in the lower 5 bits of register rs2.

Implementation  
x\[rd\] = x\[rs1\] &lt;&lt; x\[rs2\]

# slt

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>010</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
slt rd,rs1,rs2

Description  
Place the value 1 in register rd if register rs1 is less than register
rs2 when both are treated as signed numbers, else 0 is written to rd.

Implementation  
x\[rd\] = x\[rs1\] &lt;s x\[rs2\]

# sltu

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>011</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sltu rd,rs1,rs2

Description  
Place the value 1 in register rd if register rs1 is less than register
rs2 when both are treated as unsigned numbers, else 0 is written to rd.

Implementation  
x\[rd\] = x\[rs1\] &lt;u x\[rs2\]

# xor

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>100</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
xor rd,rs1,rs2

Description  
Performs bitwise XOR on registers rs1 and rs2 and place the result in rd

Implementation  
x\[rd\] = x\[rs1\] ^ x\[rs2\]

# srl

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>101</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
srl rd,rs1,rs2

Description  
Logical right shift on the value in register rs1 by the shift amount
held in the lower 5 bits of register rs2

Implementation  
x\[rd\] = x\[rs1\] &gt;&gt;u x\[rs2\]

# sra

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>01000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>101</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sra rd,rs1,rs2

Description  
Performs arithmetic right shift on the value in register rs1 by the
shift amount held in the lower 5 bits of register rs2

Implementation  
x\[rd\] = x\[rs1\] &gt;&gt;s x\[rs2\]

# or

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>110</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
or rd,rs1,rs2

Description  
Performs bitwise OR on registers rs1 and rs2 and place the result in rd

Implementation  
x\[rd\] = x\[rs1\] | x\[rs2\]

# and

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>rs2</td>
<td>rs1</td>
<td>111</td>
<td>rd</td>
<td>01100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
and rd,rs1,rs2

Description  
Performs bitwise AND on registers rs1 and rs2 and place the result in rd

Implementation  
x\[rd\] = x\[rs1\] & x\[rs2\]

# fence

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-28</td>
<td>27-24</td>
<td>23-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>0000</td>
<td>pred</td>
<td>succ</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>00011</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
fence pred, succ

Description  
Used to order device I/O and memory accesses as viewed by other RISC-V
harts and external devices or coprocessors.  
Any combination of device input (I), device output (O), memory reads
(R), and memory writes (W) may be ordered with respect to any
combination of the same.  
Informally, no other RISC-V hart or external device can observe any
operation in the successor set following a FENCE before any operation in
the predecessor set preceding the FENCE.

Implementation  
Fence(pred, succ)

# fence.i

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>00000</td>
<td>00000</td>
<td>001</td>
<td>00000</td>
<td>00011</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
fence.i

Description  
Provides explicit synchronization between writes to instruction memory
and instruction fetches on the same hart.

Implementation  
Fence(Store, Fetch)

# csrrw

atomic read/write CSR.

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">csr</td>
<td>rs1</td>
<td>001</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
csrrw rd,offset,rs1

Description  
Atomically swaps values in the CSRs and integer registers.  
CSRRW reads the old value of the CSR, zero-extends the value to XLEN
bits, then writes it to integer register rd.  
The initial value in rs1 is written to the CSR.  
If rd=x0, then the instruction shall not read the CSR and shall not
cause any of the side effects that might occur on a CSR read.

Implementation  
t = CSRs\[csr\]; CSRs\[csr\] = x\[rs1\]; x\[rd\] = t

# csrrs

atomic read and set bits in CSR.

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">csr</td>
<td>rs1</td>
<td>010</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
csrrs rd,offset,rs1

Description  
Reads the value of the CSR, zero-extends the value to XLEN bits, and
writes it to integer register rd.  
The initial value in integer register rs1 is treated as a bit mask that
specifies bit positions to be set in the CSR.  
Any bit that is high in rs1 will cause the corresponding bit to be set
in the CSR, if that CSR bit is writable.  
Other bits in the CSR are unaffected (though CSRs might have side
effects when written).

Implementation  
t = CSRs\[csr\]; CSRs\[csr\] = t | x\[rs1\]; x\[rd\] = t

# csrrc

atomic read and clear bits in CSR.

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">csr</td>
<td>rs1</td>
<td>011</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
csrrc rd,offset,rs1

Description  
Reads the value of the CSR, zero-extends the value to XLEN bits, and
writes it to integer register rd.  
The initial value in integer register rs1 is treated as a bit mask that
specifies bit positions to be cleared in the CSR.  
Any bit that is high in rs1 will cause the corresponding bit to be
cleared in the CSR, if that CSR bit is writable.  
Other bits in the CSR are unaffected.

Implementation  
t = CSRs\[csr\]; CSRs\[csr\] = t &∼x\[rs1\]; x\[rd\] = t

# csrrwi

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">csr</td>
<td>uimm</td>
<td>101</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
csrrwi rd,offset,uimm

Description  
Update the CSR using an XLEN-bit value obtained by zero-extending a
5-bit unsigned immediate (uimm\[4:0\]) field encoded in the rs1 field.

Implementation  
x\[rd\] = CSRs\[csr\]; CSRs\[csr\] = zimm

# csrrsi

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">csr</td>
<td>uimm</td>
<td>110</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
csrrsi rd,offset,uimm

Description  
Set CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit
unsigned immediate (uimm\[4:0\]) field encoded in the rs1 field.

Implementation  
t = CSRs\[csr\]; CSRs\[csr\] = t | zimm; x\[rd\] = t

# csrrci

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">csr</td>
<td>rs1</td>
<td>111</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
csrrci rd,offset,uimm

Description  
Clear CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit
unsigned immediate (uimm\[4:0\]) field encoded in the rs1 field.

Implementation  
t = CSRs\[csr\]; CSRs\[csr\] = t &∼zimm; x\[rd\] = t

# ecall

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>00000</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
ecall

Description  
Make a request to the supporting execution environment.  
When executed in U-mode, S-mode, or M-mode, it generates an
environment-call-from-U-mode exception, environment-call-from-S-mode
exception, or environment-call-from-M-mode exception, respectively, and
performs no other operation.

Implementation  
RaiseException(EnvironmentCall)

# ebreak

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>00001</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
ebreak

Description  
Used by debuggers to cause control to be transferred back to a debugging
environment.  
It generates a breakpoint exception and performs no other operation.

Implementation  
RaiseException(Breakpoint)

# uret

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00000</td>
<td>00</td>
<td>00010</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
uret

Description  
Return from traps in U-mode, and URET copies UPIE into UIE, then sets
UPIE.

Implementation  
ExceptionReturn(User)

# sret

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00010</td>
<td>00</td>
<td>00010</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sret

Description  
Return from traps in S-mode, and SRET copies SPIE into SIE, then sets
SPIE.

Implementation  
ExceptionReturn(User)

# mret

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00110</td>
<td>00</td>
<td>00010</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
mret

Description  
Return from traps in M-mode, and MRET copies MPIE into MIE, then sets
MPIE.

Implementation  
ExceptionReturn(Machine)

# wfi

wait for interrupt.

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00010</td>
<td>00</td>
<td>00101</td>
<td>00000</td>
<td>000</td>
<td>00000</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
wfi

Description  
Provides a hint to the implementation that the current hart can be
stalled until an interrupt might need servicing.  
Execution of the WFI instruction can also be used to inform the hardware
platform that suitable interrupts should preferentially be routed to
this hart.  
WFI is available in all privileged modes, and optionally available to
U-mode.  
This instruction may raise an illegal instruction exception when TW=1 in
mstatus.

Implementation  
while (noInterruptsPending) idle

# sfence.vma

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td>00010</td>
<td>01</td>
<td>rs2</td>
<td>rs1</td>
<td>000</td>
<td>rd</td>
<td>11100</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sfence.vma rs1,rs2

Description  
Guarantees that any previous stores already visible to the current
RISC-V hart are ordered before all subsequent implicit references from
that hart to the memory-management data structures.  
The SFENCE.VMA is used to flush any local hardware caches related to
address translation.  
It is specified as a fence rather than a TLB flush to provide cleaner
semantics with respect to which instructions are affected by the flush
operation and to support a wider variety of dynamic caching structures
and memory-management schemes.  
SFENCE.VMA is also used by higher privilege levels to synchronize page
table writes and the address translation hardware.

Implementation  
Fence(Store, AddressTranslation)

# lb

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[11:0]</td>
<td>rs1</td>
<td>000</td>
<td>rd</td>
<td>00000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
lb rd,offset(rs1)

Description  
Loads a 8-bit value from memory and sign-extends this to XLEN bits
before storing it in register rd.

Implementation  
x\[rd\] = sext(M\[x\[rs1\] + sext(offset)\]\[7:0\])

# lh

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[11:0]</td>
<td>rs1</td>
<td>001</td>
<td>rd</td>
<td>00000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
lh rd,offset(rs1)

Description  
Loads a 16-bit value from memory and sign-extends this to XLEN bits
before storing it in register rd.

Implementation  
x\[rd\] = sext(M\[x\[rs1\] + sext(offset)\]\[15:0\])

# lw

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[11:0]</td>
<td>rs1</td>
<td>010</td>
<td>rd</td>
<td>00000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
lw rd,offset(rs1)

Description  
Loads a 32-bit value from memory and sign-extends this to XLEN bits
before storing it in register rd.

Implementation  
x\[rd\] = sext(M\[x\[rs1\] + sext(offset)\]\[31:0\])

# lbu

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[11:0]</td>
<td>rs1</td>
<td>100</td>
<td>rd</td>
<td>00000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
lbu rd,offset(rs1)

Description  
Loads a 8-bit value from memory and zero-extends this to XLEN bits
before storing it in register rd.

Implementation  
x\[rd\] = M\[x\[rs1\] + sext(offset)\]\[7:0\]

# lhu

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[11:0]</td>
<td>rs1</td>
<td>101</td>
<td>rd</td>
<td>00000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
lhu rd,offset(rs1)

Description  
Loads a 16-bit value from memory and zero-extends this to XLEN bits
before storing it in register rd.

Implementation  
x\[rd\] = M\[x\[rs1\] + sext(offset)\]\[15:0\]

# sb

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:74%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 9%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 16%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[11:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>000</td>
<td>offset[4:0]</td>
<td>01000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sb rs2,offset(rs1)

Description  
Store 8-bit, values from the low bits of register rs2 to memory.

Implementation  
M\[x\[rs1\] + sext(offset)\] = x\[rs2\]\[7:0\]

# sh

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:74%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 9%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 16%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[11:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>001</td>
<td>offset[4:0]</td>
<td>01000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sh rs2,offset(rs1)

Description  
Store 16-bit, values from the low bits of register rs2 to memory.

Implementation  
M\[x\[rs1\] + sext(offset)\] = x\[rs2\]\[15:0\]

# sw

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:74%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 9%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 16%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[11:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>010</td>
<td>offset[4:0]</td>
<td>01000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
sw rs2,offset(rs1)

Description  
Store 32-bit, values from the low bits of register rs2 to memory.

Implementation  
M\[x\[rs1\] + sext(offset)\] = x\[rs2\]\[31:0\]

# jal

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[20<a href="##SUBST##|10:1|">|10:1|</a>11</td>
<td colspan="2">19:12]</td>
<td>rd</td>
<td>11011</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
jal rd,offset

Description  
Jump to address and place return address in rd.

Implementation  
x\[rd\] = pc+4; pc += sext(offset)

# jalr

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:64%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="3">offset[11:0]</td>
<td>rs1</td>
<td>000</td>
<td>rd</td>
<td>11001</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
jalr rd,rs1,offset

Description  
Jump to address and place return address in rd.

Implementation  
t =pc+4; pc=(x\[rs1\]+sext(offset))&∼1; x\[rd\]=t

# beq

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:82%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 13%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 20%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[12|10:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>000</td>
<td>offset[4:1|11]</td>
<td>11000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
beq rs1,rs2,offset

Description  
Take the branch if registers rs1 and rs2 are equal.

Implementation  
if (x\[rs1\] == x\[rs2\]) pc += sext(offset)

# bne

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:82%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 13%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 20%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[12|10:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>001</td>
<td>offset[4:1|11]</td>
<td>11000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
bne rs1,rs2,offset

Description  
Take the branch if registers rs1 and rs2 are not equal.

Implementation  
if (x\[rs1\] != x\[rs2\]) pc += sext(offset)

# blt

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:82%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 13%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 20%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[12|10:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>100</td>
<td>offset[4:1|11]</td>
<td>11000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
blt rs1,rs2,offset

Description  
Take the branch if registers rs1 is less than rs2, using signed
comparison.

Implementation  
if (x\[rs1\] &lt;s x\[rs2\]) pc += sext(offset)

# bge

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:82%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 13%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 20%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[12|10:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>101</td>
<td>offset[4:1|11]</td>
<td>11000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
bge rs1,rs2,offset

Description  
Take the branch if registers rs1 is greater than or equal to rs2, using
signed comparison.

Implementation  
if (x\[rs1\] &gt;=s x\[rs2\]) pc += sext(offset)

# bltu

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:82%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 13%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 20%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[12|10:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>110</td>
<td>offset[4:1|11]</td>
<td>11000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
bltu rs1,rs2,offset

Description  
Take the branch if registers rs1 is less than rs2, using unsigned
comparison.

Implementation  
if (x\[rs1\] &lt;u x\[rs2\]) pc += sext(offset)

# bgeu

[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c[|c|](##SUBST##|c|)c|

<table style="width:82%;">
<colgroup>
<col style="width: 8%" />
<col style="width: 13%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 8%" />
<col style="width: 20%" />
<col style="width: 8%" />
<col style="width: 5%" />
</colgroup>
<tbody>
<tr>
<td>31-27</td>
<td>26-25</td>
<td>24-20</td>
<td>19-15</td>
<td>14-12</td>
<td>11-7</td>
<td>6-2</td>
<td>1-0</td>
</tr>
<tr>
<td colspan="2">offset[12|10:5]</td>
<td>rs2</td>
<td>rs1</td>
<td>111</td>
<td>offset[4:1|11]</td>
<td>11000</td>
<td>11</td>
</tr>
</tbody>
</table>

Format  
bgeu rs1,rs2,offset

Description  
Take the branch if registers rs1 is greater than or equal to rs2, using
unsigned comparison.

Implementation  
if (x\[rs1\] &gt;=u x\[rs2\]) pc += sext(offset)
