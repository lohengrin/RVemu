#!/usr/bin/env python3
"""
Simple ELF file generator for testing RVemu ElfLoader.
Creates minimal RV64 ELF executables with specific characteristics.
"""

import struct
import sys

class ELFWriter:
    def __init__(self):
        self.data = bytearray()
        
    def write_byte(self, val):
        self.data.append(val & 0xFF)
        
    def write_half(self, val):
        self.data.extend(struct.pack('<H', val))
        
    def write_word(self, val):
        self.data.extend(struct.pack('<I', val))
        
    def write_xword(self, val):
        self.data.extend(struct.pack('<Q', val))
        
    def write_bytes(self, val):
        self.data.extend(val)

def create_simple_elf(output_file, entry_point=0x80000000):
    """Create a minimal RV64 ELF executable with simple code."""
    writer = ELFWriter()
    
    # ELF Header
    writer.write_bytes(b'\x7fELF')  # Magic
    writer.write_byte(2)  # 64-bit
    writer.write_byte(1)  # Little endian
    writer.write_byte(1)  # ELF version
    writer.write_byte(0)  # System V ABI
    writer.write_byte(0)  # ABI version
    writer.write_bytes(b'\x00' * 7)  # Padding
    
    writer.write_half(2)  # e_type: ET_EXEC
    writer.write_half(243)  # e_machine: EM_RISCV
    writer.write_half(1)  # e_version
    
    # Calculate entry point (will be set after segment layout)
    entry_offset = len(writer.data)
    writer.write_xword(entry_point)
    
    # Program header offset (after ELF header)
    e_phoff = 64
    writer.write_xword(e_phoff)
    
    # Section header offset (set to 0 for simplicity, no sections)
    e_shoff = 0
    writer.write_xword(e_shoff)
    
    writer.write_word(0)  # e_flags
    writer.write_half(64)  # e_ehsize (64 bytes for ELF64)
    writer.write_half(56)  # e_phentsize
    writer.write_half(2)  # e_phnum
    writer.write_half(64)  # e_shentsize
    writer.write_half(0)  # e_shnum (no section headers)
    writer.write_half(0)  # e_shstrndx (no section string table)
    
    # Pad to 64 bytes
    while len(writer.data) < 64:
        writer.write_byte(0)
    
    # Program headers start here
    
    # Program header 1: PT_LOAD (code segment)
    # p_type: PT_LOAD
    writer.write_xword(1)       
    # p_flags: PF_X | PF_R
    writer.write_xword(5)       
    # p_offset: offset in file (right after program headers)
    p_offset = 64 + 2*56
    writer.write_xword(p_offset)
    # p_vaddr: virtual address (DRAM_BASE aligned)
    p_vaddr = entry_point
    writer.write_xword(p_vaddr)
    # p_paddr: physical address (same as virtual)
    writer.write_xword(p_vaddr)
    # p_filesz: file size (simple instruction)
    simple_code = b'\x93\x08\xd0\x93'  # addi x0, x0, 0x13 (nop-like)
    p_filesz = len(simple_code)
    writer.write_xword(p_filesz)
    # p_memsz: memory size (same as file size)
    writer.write_xword(p_filesz)
    # p_align: 4KB page alignment
    writer.write_xword(0x1000)
    
    # Program header 2: PT_LOAD (data segment with BSS)
    # p_type: PT_LOAD
    writer.write_xword(1)       
    # p_flags: PF_W | PF_R
    writer.write_xword(6)       
    # p_offset: offset in file (after code)
    p_offset2 = p_offset + p_filesz
    writer.write_xword(p_offset2)
    # p_vaddr: virtual address (aligned to next page)
    p_vaddr2 = ((p_vaddr + p_filesz + 0xFFF) & ~0xFFF)
    writer.write_xword(p_vaddr2)
    # p_paddr: physical address
    writer.write_xword(p_vaddr2)
    # p_filesz: file size (some initialized data)
    data_content = b'\x42\x00\x00\x00\x00\x00\x00\x00'  # 64-bit value 0x42
    p_filesz2 = len(data_content)
    writer.write_xword(p_filesz2)
    # p_memsz: memory size (larger to create BSS)
    p_memsz2 = p_filesz2 + 0x1000  # 4KB BSS
    writer.write_xword(p_memsz2)
    # p_align: 4KB page alignment
    writer.write_xword(0x1000)
    
    # Code segment data
    writer.write_bytes(simple_code)
    
    # Data segment data
    writer.write_bytes(data_content)
    
    with open(output_file, 'wb') as f:
        f.write(writer.data)

def create_invalid_elf(output_file):
    """Create an invalid ELF file for error testing."""
    with open(output_file, 'wb') as f:
        f.write(b'\x7fNOTANELF' + b'\x00' * 100)

def create_32bit_elf(output_file):
    """Create a 32-bit ELF (should be rejected)."""
    writer = ELFWriter()
    
    # ELF Header for 32-bit
    writer.write_bytes(b'\x7fELF')  # Magic
    writer.write_byte(1)  # 32-bit (this should cause rejection)
    writer.write_byte(1)  # Little endian
    writer.write_byte(1)  # ELF version
    writer.write_bytes(b'\x00' * 8)  # Rest of header fields
    
    writer.write_half(0x0002)  # e_type: ET_EXEC
    writer.write_half(0xf3)    # e_machine: EM_RISCV
    writer.write_word(1)       # e_version
    writer.write_word(0x10000)  # e_entry
    
    with open(output_file, 'wb') as f:
        f.write(writer.data)

def create_big_endian_elf(output_file):
    """Create a big-endian ELF (should be rejected)."""
    writer = ELFWriter()
    
    # ELF Header
    writer.write_bytes(b'\x7fELF')  # Magic
    writer.write_byte(2)  # 64-bit
    writer.write_byte(2)  # Big endian (this should cause rejection)
    writer.write_byte(1)  # ELF version
    writer.write_bytes(b'\x00' * 8)  # Rest of header fields
    
    writer.write_half(0x0002)  # e_type: ET_EXEC
    writer.write_half(0xf3)    # e_machine: EM_RISCV
    writer.write_word(1)       # e_version
    writer.write_xword(0x80000000)  # e_entry
    
    with open(output_file, 'wb') as f:
        f.write(writer.data)

if __name__ == '__main__':
    import os
    
    # Create test directory
    test_dir = 'test_elfs'
    os.makedirs(test_dir, exist_ok=True)
    
    print("Creating test ELF files...")
    
    # Valid ELF
    create_simple_elf(os.path.join(test_dir, 'valid_64bit.elf'), entry_point=0x80000000)
    print("Created: valid_64bit.elf")
    
    # Invalid ELF
    create_invalid_elf(os.path.join(test_dir, 'invalid_magic.elf'))
    print("Created: invalid_magic.elf")
    
    # 32-bit ELF (should be rejected)
    create_32bit_elf(os.path.join(test_dir, '32bit.elf'))
    print("Created: 32bit.elf")
    
    # Big-endian ELF (should be rejected)
    create_big_endian_elf(os.path.join(test_dir, 'bigendian.elf'))
    print("Created: bigendian.elf")
    
    print(f"\nTest ELF files created in '{test_dir}/' directory")