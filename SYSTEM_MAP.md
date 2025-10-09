# SYSTEM_MAP.md
## Modules and their purposes
Based on the README analysis, this is an experimental educational implementation of Apple's macOS System 7 architecture with a focus on reverse engineering education. The system implements core components for modern x86 hardware booting through GRUB/Multiboot2.
**Core Components:**
- **Boot System**: Handles platform-independent boot dispatch via `hal_boot_init()` 
- **Serial Logging System**: Module-based logging with runtime filtering and hierarchical levels (Error, Warn, Info, Debug, Trace)
- **Graphics Foundation**: VESA framebuffer (800x600x32) with QuickDraw primitives
- **Desktop Rendering**: System 7 menu bar with Apple logo, desktop pattern rendering, hard drive icon with proper rendering and composing
- **Typography**: Chicago bitmap font with pixel-perfect rendering and proper kerning
- **Font Manager**: Full System 7.1-compatible implementation with multiple font sizes (9-24pt) with nearest-neighbor scaling, style synthesis (bold, italic, underline, shadow, outline, condense/extend), FOND/NFNT resource parsing for bitmap fonts
- **Input System**: PS/2 keyboard and mouse support with full event forwarding piping
- **Event Manager**: Cooperative multitasking via WaitNextEvent, unified event queue with mouse/keyboard event dispatching, proper event mask filtering and sleep/idle support
- **Resource System**: Pattern (PAT) and pixel pattern (ppat) resources from JSON
- **Icon System**: Small icons (SICON) and 32-bit color icons (ARG) with mask composition
- **Memory Manager**: Zone-based memory management (8MB total: System and App zones)
- **Menu Manager**: Complete menu system with full dropdown functionality
- **File System**: Virtual file system with B-tree implementation and trash folder integration  
- **Window Manager**: Window structure, creation, and display with chrome (title bar, close box)
- **Time Manager**: Production-quality implementation with accurate TSC frequency calibration, binary min-heap scheduler with O(log n) operations
- **OSUtils Traps**: `_TickCount` trap service by Time Manager-backed periodic task for 68K code
- **Resource Manager**: High-performance implementation with binary search index for O(log n) lookups, LRU cache with open-addressing hash table, comprehensive bounds checking and validation, named resource support (GetNamedResource)
**Subsystems Not Yet Compiled:**
- AppleEventManager (8 files): Inter-application messaging and scripting support
- DeviceManager (8 files): Device driver infrastructure  
- FontResources (8 files): Font resource management and conversion
- GestaltManager (2 files): Extended system capability detection
- SpeechManager (8 files): Text-to-speech synthesis
- StartupScreen (1 file): Boot splash screen
## Dependencies and build targets
### Build System
- GNU Make
- GCC with 32-bit support (`gcc-multilib` on 64-bit systems)
- GRUB tools (`grub-mkrescue` from `grub-common` or `grub-pc-bin`)
- QEMU for testing (`qemu-system-i386`)
- Python 3 for resource processing
- xxd for binary conversion
### Platform Support
- Multi-platform support via Hardware Abstraction Layer (HAL)
- Platform-agnostic core implementation
- x86 platform complete (ATA, PS/2, VGA support)
- Ready for ARM (Raspberry Pi, Apple Silicon) and PowerPC ports
- Build system with `make PLATFORM=<platform>` (default: x86)
### Key Subsystems Architecture
```
src/
├── main.c              # Kernel entry point & initialization
├── boot.c              # Platform-independent boot dispatcher
├── SystemInit.c        # System startup sequence
├── System7StdLib.c     # Serial logging framework with bracket tag parsing
├── Platform/           # Hardware Abstraction Layer (HAL)
├── include/            # Public headers (Inside Mac API)
├── docs/               # Component documentation  
├── ControlManager/     # UI controls (Core, Tracking, StandardControls, Scrollbar)
├── DialogManager/      # Dialog boxes (Core, Keyboard, StandardFile integration)
├── FontManager/        # Font management (5 modules)
├── EventManager/       # Event handling & cooperative multitasking
├── MemoryMgr/          # Zone-based memory management
├── Finder/             # Desktop & Finder implementation
├── Icon/               # Icon system (5 modules)
├── FS/                 # Virtual file system & HFS
├── ControlManager/     # UI controls (4 modules: Core, Tracking, StandardControls, Scrollbar)
├── DialogManager/      # Dialog boxes (3 modules: Core, Keyboard, StandardFile integration)
├── DeskManager/        # Desktop accessories
├── PatternMgr/         # Pattern resources
├── TimeManager/        # Production timer scheduler (7 modules)
├── ResourceManager/    # High-performance resource manager
├── Gestalt/            # System information manager
├── TextEdit/           # TextEdit Manager (7 modules)
├── ScrapManager/       # Clipboard/Scrap Manager (Classic Mac OS API)
├── ListManager/        # List Manager (3 modules)  
├── Apps/SimpleText/    # SimpleText application (7 modules)
├── SegmentLoader/      # Portable 68K segment loader (3 modules)
├── CPU/                # CPU backend interface & implementations
├── m68k_interpret/     # M68K interpreter backend
├── ProcessMgr/         # Process Manager (integrated with segment loader)
├── FileMgr/            # File Manager subsystems
└── include/            # Public headers (Inside Mac API)
```
### Health Status
✅ OK - Core functionality is partially working with many features implemented but not fully integrated or compiled. The system includes the core architecture and major components.
## External libraries and versions
- GCC compiler toolchain with multilib support for 32-bit compilation
- GRUB bootloader (multiboot2 support)
- QEMU emulator for testing 
- Python 3 for resource processing scripts
- xxd binary converter utility
- Standard C library (glibc)