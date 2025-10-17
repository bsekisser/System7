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

  ." Attempting to boot System 7 kernel from disk..." cr
  cr

  \ Use OF 'boot' command which properly initializes client interface
  \ This sets up r5 (OF callback), r3/r4 (initrd), and jumps to entry point
  \ Unlike 'load/go', 'boot' is the standard ELF client boot path
  boot hd:,\kernel.elf
;

\ Run boot sequence automatically
boot-system
