OUTPUT_FORMAT("elf64-littleriscv")
ENTRY(_start)

MEMORY {
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS {
  .text : {
    *(.text*)
  } > RAM :text

  .data : {
    *(.data*)
    *(.rodata*)
  } > RAM :data

  .bss : {
    *(.bss*)
    *(COMMON)
  } > RAM :data

  . = ALIGN(8);
  _end = .;
}

PHDRS {
  text PT_LOAD FLAGS(5);  /* PF_R | PF_X */
  data PT_LOAD FLAGS(6);  /* PF_R | PF_W */
}