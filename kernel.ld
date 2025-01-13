ENTRY(boot)

SECTIONS {
  . = 0x80200000;

  .text : {
    KEEP(*(.text.boot));
    *(.text .text.*);
  }

  .rodata : ALIGN(4) {
    *(.rodata .rodata.*);
  }

  .data : ALIGN(4) {
    *(.data .data.*);
  }

  .bss : ALIGN(4) {
    __bss = .;
    *(.bss .bss.* .sbss .sbss.*);
    __bss_end = .;
  }
  
  /* stack allocation */
  . =  ALIGN(4);
  . += 128 * 1024; /* 128 KB */
  __stack_top = .;

  /* heap allocation */
  . = ALIGN(4096); /* A page = 4KB */
  __heap_start = .;
  . += 64 * 1024 * 1024; /* 64 MB */
  __heap_end = .;
}
