\ Open Firmware Boot Loader - Raw Binary Method
\
\ This loader bypasses OpenBIOS's incomplete ELF client support
\ by loading a raw binary kernel image and executing it directly.
\
\ The approach:
\ 1. Load kernel.bin from disk to memory at 0x01000000
\ 2. Jump to entry point with proper OF context preserved
\
\ This method is more reliable than ELF loading on firmware with
\ incomplete client-program support.

0h 01000000 constant KERNEL_ADDR
0h 04000000 constant LOAD_SIZE        \ 64 MB max

\ Open the kernel binary file from disk
: open-kernel-file ( -- fd )
  " hd:,\kernel.bin" open-dev
;

\ Read kernel binary into memory
: load-kernel-binary ( -- size )
  " hd:,\kernel.bin" $read-file    \ Read file into a buffer
  nip                              \ Drop buffer address, keep size
;

\ Display boot information
: show-boot-info
  ." ================================================" cr
  ." System 7 Portable - Raw Binary Boot" cr
  ." Open Firmware Bootloader" cr
  ." ================================================" cr
  cr
  ." Loading kernel binary..." cr
  ." Entry point: " KERNEL_ADDR hex . decimal cr
  cr
;

\ Execute the loaded kernel
\ Note: When this returns (or jumps), we should never get here
: execute-kernel
  ." Transferring control to kernel..." cr
  cr
  \ Jump to kernel entry point
  \ The kernel stub (platform_boot.S) will:
  \ 1. Receive r5 = OF callback (preserved from here)
  \ 2. Set up its own stack and BSS
  \ 3. Call boot_main()
  KERNEL_ADDR execute
;

\ Main boot sequence
: boot-system
  show-boot-info

  \ Attempt to load kernel binary from disk
  ['] open-kernel-file catch if
    ." Error: Cannot open kernel.bin" cr
    ." Please ensure kernel.bin is on the boot disk" cr
    ." Returning to firmware prompt." cr
    exit
  then
  drop

  \ Load binary to kernel address
  KERNEL_ADDR
  " hd:,\kernel.bin" $read-file
  drop

  ." ✓ Kernel loaded to " KERNEL_ADDR hex . decimal cr
  ." ✓ Size: " . ." bytes" cr
  cr

  \ Execute kernel - never returns
  execute-kernel
;

\ Run boot sequence
boot-system
