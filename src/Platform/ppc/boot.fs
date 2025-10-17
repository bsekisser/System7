\ Open Firmware Forth Bootloader for System 7 Portable
\
\ Automatic boot via QEMU -device loader mechanism
\ When QEMU loads kernel via -device loader,file=kernel.elf,addr=0x01000000
\ This script detects it and automatically jumps to it
\
\ BOOT METHODS:
\   1. QEMU with -device loader: Automatic boot (this script)
\   2. Manual from OF prompt: load hd:2,\\kernel.elf then go
\

\ Define kernel address (where it will be loaded by QEMU -device loader)
01000000 constant KERNEL_ADDR

\ Magic number check for valid ELF header
\ ELF files start with 0x7f 'E' 'L' 'F'
7f constant ELF_MAGIC1
45 constant ELF_MAGIC_E
4c constant ELF_MAGIC_L
46 constant ELF_MAGIC_F

\ Check if valid ELF kernel is at KERNEL_ADDR
: is-kernel-present ( -- flag )
  KERNEL_ADDR @ dup 0= if
    drop false
  else
    \ Check ELF magic: 0x7f 'E' 'L' 'F'
    dup c@ ELF_MAGIC1 = if
      1+ dup c@ ELF_MAGIC_E = if
        1+ dup c@ ELF_MAGIC_L = if
          1+ c@ ELF_MAGIC_F =
        then
      then
    then
  then
;

\ Automatic boot attempt
: boot-system
  ." ================================================" cr
  ." System 7 Portable - Open Firmware Boot" cr
  ." IEEE 1275 Bootloader" cr
  ." ================================================" cr
  cr

  ." Checking for preloaded kernel..." cr

  is-kernel-present if
    ." ✓ Kernel found at 0x01000000" cr
    ." ✓ ELF magic verified" cr
    cr
    ." Transferring control to kernel..." cr
    cr
    KERNEL_ADDR go
  else
    ." Kernel not found at 0x01000000" cr
    cr
    ." BOOT OPTIONS:" cr
    ." 1. Use: make PLATFORM=ppc run" cr
    ."    (QEMU will auto-load kernel via -device loader)" cr
    cr
    ." 2. Manual boot from OF prompt:" cr
    ."    0 > load hd:2,\\kernel.elf" cr
    ."    0 > go" cr
    cr
    ." 3. Or check QEMU device configuration" cr
    cr
  then
;

\ Run boot sequence automatically
boot-system
