/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(__start)
SEARCH_DIR("=/usr/local/lib/arm-linux-gnueabihf"); SEARCH_DIR("=/usr/local/lib"); SEARCH_DIR("=/lib/arm-linux-gnueabihf"); SEARCH_DIR("=/lib"); SEARCH_DIR("=/usr/lib/arm-linux-gnueabihf"); SEARCH_DIR("=/usr/lib");
SECTIONS
{
  PROVIDE(__executable_start = 0x0008000);
  . = 0x00008000 + SIZEOF_HEADERS;
  .interp         : { *(.interp) }
  .note.ABI-tag   : { *(.note.ABI-tag) }
  .hash           : { *(.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .version        : { *(.version) }
  .version_d      : { *(.version_d) }
  .version_r      : { *(.version_r) }
  .rel.dyn        : { *(.rel.dyn) }
  .rela.dyn       : { *(.rela.dyn) }
  .rel.plt        : { *(.rel.plt) }
  .rela.plt       : { *(.rela.plt) }
  .init           : { KEEP (*(.init)) }
  .plt            : { *(.plt) }
  .text           : { *(.text .text.*) }
  .fini           : { KEEP (*(.fini)) }
  PROVIDE(__etext = .);
  PROVIDE(_etext = .);
  PROVIDE(etext = .);
  .rodata         : { *(.rodata .rodata.*) }
   __exidx_start = .;
  .ARM.exidx   : { *(.ARM.exidx*) }
   __exidx_end = .;

  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1));
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));

  .tdata	  : { *(.tdata .tdata.*) }
  .tbss		  : { *(.tbss .tbss.*) }
  .preinit_array     :
  {
     PROVIDE_HIDDEN (__preinit_array_start = .);
     KEEP (*(.preinit_array))
     PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array     :
  {
     PROVIDE_HIDDEN (__init_array_start = .);
     KEEP (*(.init_array*))
     PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array     :
  {
     PROVIDE_HIDDEN (__fini_array_start = .);
     KEEP (*(.fini_array*))
     PROVIDE_HIDDEN (__fini_array_end = .);
  }
  .dynamic        : { *(.dynamic) }
  .got            : { *(.got.plt) *(.got) }
  .data           :
  {
    __data_start = .;
    *(.data .data.*)
  }
  _edata = .;
  PROVIDE(edata = .);
  __bss_start = .;
  __bss_start__ = .;
  .bss            :
  {
   *(.bss .bss.*)
   . = ALIGN(. != 0 ? 32 / 8 : 1);
  }
  __bss_end__ = .;
  _bss_end__ = .;
  . = ALIGN(4);
  __end = .;
  _end = .;
  PROVIDE(end = .);
}
