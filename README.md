# System 7 - Iteration 2

<img width="793" height="657" alt="System 7 running on modern hardware" src="https://github.com/user-attachments/assets/be84b83e-191c-4f9d-a786-11d0bd04203b" />

> ⚠️ **PROOF OF CONCEPT** - This is an experimental, educational reimplementation of Apple's Macintosh System 7. This is NOT a finished product and should not be considered production-ready software.

An open-source reimplementation of Apple Macintosh System 7 for modern x86 hardware, bootable via GRUB2/Multiboot2. This project aims to recreate the classic Mac OS experience while documenting the System 7 architecture through reverse engineering and archaeological analysis.

## 🎯 Project Status

**Current State**: Active development, core functionality partially working

This is a proof-of-concept implementation focused on understanding and recreating System 7's architecture. Many features are incomplete, stubbed, or in various stages of development.

### What Works ✅

- **Boot System**: Successfully boots via GRUB2/Multiboot2 on x86 hardware
- **Graphics Foundation**: VESA framebuffer (800x600x32) with QuickDraw primitives
- **Desktop Rendering**:
  - System 7 menu bar with rainbow Apple logo
  - Desktop pattern rendering with authentic System 7 patterns
  - Hard drive icon with proper rendering and compositing
  - Trash icon support
- **Typography**: Chicago bitmap font with pixel-perfect rendering and proper kerning
- **Input System**: PS/2 keyboard and mouse support with event handling
- **Event Manager**: Unified event queue with mouse/keyboard event dispatching
- **Resource System**: Pattern (PAT) and pixel pattern (ppat) resources from JSON
- **Icon System**: Small icons (SICN) and 32-bit color icons (ARGB) with mask compositing
- **Memory Manager**: Zone-based memory management (8MB total: System and App zones)
- **Menu System**: Menu bar rendering with File, Edit, View, and Label menus
- **File System**: HFS virtual file system with B-tree implementation and trash folder integration
- **Window Manager Core**: Window structure, basic display, and event handling (see limitations below)

### Partially Working ⚠️

- **Window Manager**:
  - Window structure and creation works
  - Window display with chrome (title bar, close box) renders
  - **BROKEN**: Window dragging is currently non-functional ([see commit e6a4ef8](https://github.com/user-attachments/commit/e6a4ef8))
  - Window content rendering has coordinate system issues
  - Update event pipeline incomplete
  - WDEF (Window Definition Procedure) dispatch not implemented
  - Visible region (visRgn) calculation incomplete
- **Desktop Icons**:
  - Icons render and display correctly
  - Icon dragging implemented but has visual artifacts
  - Double-click to open partially functional
- **Menu Manager**:
  - Menu bar displays and tracks mouse correctly
  - Dropdown menu rendering incomplete
  - Menu selection and command dispatch stubbed
- **Control Manager**: Framework in place, most controls not implemented
- **Dialog Manager**: Core structure present, dialog drawing/interaction not complete
- **File Manager**: API stubs present, full HFS operations incomplete

### Not Yet Implemented ❌

- **Application Launching**: No process management or application loading
- **Clipboard/Scrap Manager**: Copy/paste functionality absent
- **Resource Manager**: Full .rsrc file support not implemented
- **TextEdit**: Text editing system not functional
- **List Manager**: List controls not working
- **Sound Manager**: No audio support
- **Printing**: No print system
- **Networking**: No AppleTalk or network functionality
- **Desk Accessories**: Framework only, no functional DAs
- **Color Manager**: Minimal color support
- **Font Manager**: Only Chicago font supported, no TrueType or font scaling

## 🏗️ Architecture

### Technical Specifications

- **Architecture**: 32-bit x86 (i386)
- **Boot Protocol**: Multiboot2
- **Graphics**: VESA framebuffer, 800x600 @ 32-bit color
- **Memory Layout**: Kernel loads at 1MB physical address
- **Font Rendering**: Custom bitmap renderer with authentic Chicago font data
- **Input**: PS/2 keyboard and mouse via port I/O
- **Toolbox Managers**: Modular implementation matching Inside Macintosh specifications

### Codebase Statistics

- **138 header files** across 24+ subsystems
- **213 source files** (~50,000+ lines of code)
- **69 resource types** extracted from original System 7.1
- **57 documented archaeological findings** with Inside Macintosh citations

### Key Subsystems

```
iteration2/
├── src/
│   ├── main.c                  # Kernel entry point & initialization
│   ├── SystemInit.c            # System startup sequence
│   ├── QuickDraw/              # 2D graphics primitives
│   ├── WindowManager/          # Window management (8 modules)
│   ├── MenuManager/            # Menu system (7 modules)
│   ├── EventManager/           # Event handling (6 modules)
│   ├── MemoryMgr/              # Zone-based memory management
│   ├── Finder/                 # Desktop & Finder implementation
│   │   └── Icon/               # Icon system (5 modules)
│   ├── FS/                     # Virtual file system & HFS
│   ├── ControlManager/         # UI controls
│   ├── DialogManager/          # Dialog boxes
│   ├── DeskManager/            # Desk accessories
│   ├── PatternMgr/             # Pattern resources
│   ├── FileManager.c           # File Manager API
│   └── PS2Controller.c         # Hardware input driver
├── include/                    # Public headers (Inside Mac API)
├── docs/                       # Archaeological documentation
│   ├── ARCHAEOLOGY.md          # 57 documented findings with citations
│   ├── WM_STATUS_REPORT.md     # Window Manager implementation status
│   └── WM_RESTRUCTURE_PLAN.md  # Architecture planning docs
└── System_Resources_Extracted/ # Original System 7.1 resources
    └── [69 resource type folders]
```

## 🔨 Building

### Requirements

- **GCC** with 32-bit support (`gcc-multilib` on 64-bit systems)
- **GNU Make**
- **GRUB tools**: `grub-mkrescue` (from `grub2-common` or `grub-pc-bin`)
- **QEMU** for testing (`qemu-system-i386`)
- **Python 3** for resource processing
- **xxd** for binary conversion

### Ubuntu/Debian Installation

```bash
sudo apt-get install build-essential gcc-multilib grub-pc-bin xorriso qemu-system-x86 python3 vim-common
```

### Build Commands

```bash
# Build kernel only
make

# Create bootable ISO
make iso

# Build and run in QEMU
make run

# Clean build artifacts
make clean

# Check exported symbols
make check-exports

# Display build statistics
make info
```

## 🚀 Running

### Quick Start (QEMU)

```bash
# Standard run with serial logging
make run

# Or manually:
qemu-system-i386 -cdrom system71.iso -serial file:/tmp/serial.log -display sdl -vga std -m 256M
```

### Run Options

```bash
# Debug with GDB
make debug
# In another terminal: gdb kernel.elf -ex "target remote :1234"

# Custom QEMU options
qemu-system-i386 -cdrom system71.iso \
  -serial stdio \           # Serial to console
  -display sdl \            # SDL window
  -vga std \                # Standard VGA
  -m 256M                   # 256MB RAM
```

### Real Hardware

**⚠️ WARNING**: Running on real hardware is experimental and may not work. Use at your own risk.

1. Write ISO to USB drive: `dd if=system71.iso of=/dev/sdX bs=4M`
2. Boot from USB with legacy BIOS (not UEFI)
3. Serial output available on COM1 (115200 baud) if available

## 📚 Documentation

### For Developers

- **[ARCHAEOLOGY.md](docs/ARCHAEOLOGY.md)**: 57 documented findings with Inside Macintosh citations and archaeological evidence for every implementation decision
- **[WM_STATUS_REPORT.md](docs/WM_STATUS_REPORT.md)**: Detailed Window Manager implementation status (94% error-free)
- **[WINDOW_OPENING_ISSUE.md](WINDOW_OPENING_ISSUE.md)**: Event Manager refactoring documentation
- **[ICON_SYSTEM_IMPLEMENTATION.md](ICON_SYSTEM_IMPLEMENTATION.md)**: Icon system architecture guide
- **[docs/WM_RESTRUCTURE_PLAN.md](docs/WM_RESTRUCTURE_PLAN.md)**: Window Manager architecture planning

### Implementation Philosophy

This project follows an **archaeological approach** to reimplementation:

1. **Evidence-Based**: Every non-trivial implementation decision is backed by citations from:
   - Inside Macintosh documentation (official Apple specs)
   - MPW Universal Interfaces (canonical struct layouts)
   - System 7 ROM disassembly analysis
   - Emulator trace comparison

2. **Documented Findings**: All changes are tagged with Finding IDs (e.g., `[WM-001]`) that reference archaeological evidence in `docs/ARCHAEOLOGY.md`

3. **Behavioral Parity**: Goal is to match original System 7 behavior, not to improve or modernize it

4. **Clean Room**: Implementation is based on public documentation and black-box analysis, not original Apple source code

## 🎓 Educational Value

This project serves as:

- **Operating System Education**: Demonstrates classic OS architecture (cooperative multitasking, event-driven UI, resource management)
- **Reverse Engineering Study**: Documents the process of analyzing and recreating complex software systems
- **Historical Preservation**: Keeps System 7's architecture and design philosophy accessible
- **Low-Level Programming**: Real bare-metal x86 programming with no OS layer

## 🐛 Known Issues

1. **Window Dragging Broken**: As of commit e6a4ef8, window dragging does not work (restoration of System 7-faithful behavior broke drag functionality)
2. **Window Content Coordinate Issues**: Window content may not render in correct positions
3. **Menu Dropdowns Incomplete**: Menus display but don't show items when clicked
4. **Icon Drag Artifacts**: Dragging desktop icons causes visual artifacts
5. **No Application Support**: Cannot launch or run applications
6. **Limited Font Support**: Only Chicago font implemented
7. **HFS Read-Only**: File system is virtual/simulated, no real disk I/O
8. **No Stability Guarantees**: Crashes, hangs, and unexpected behavior are common

## 🤝 Contributing

This is primarily a learning/research project, but contributions are welcome:

1. **Bug Reports**: File issues with detailed reproduction steps
2. **Documentation**: Help improve archaeological documentation with citations
3. **Testing**: Test on different hardware/emulators and report results
4. **Implementation**: Submit PRs with proper `[WM-###]` style findings documentation

**Important**: All contributions must maintain the archaeological approach with proper citations and evidence.

## 📖 Resources

### Essential References

- **Inside Macintosh** (1992-1994 editions): Official Apple Toolbox documentation
- **MPW Universal Interfaces 3.2**: Canonical header files and struct definitions
- **Guide to Macintosh Family Hardware**: Hardware architecture reference

### Helpful Tools

- **Mini vMac**: System 7 emulator for behavioral reference
- **Basilisk II**: Another classic Mac emulator
- **ResEdit**: Resource editor (for studying original System 7 resources)
- **Ghidra/IDA**: For ROM disassembly analysis

## ⚖️ Legal

This is a **clean-room reimplementation** for **educational and preservation purposes**.

- **No Apple source code** was used in this implementation
- Implementation is based on public documentation and black-box analysis
- "System 7", "Macintosh", "QuickDraw", and Apple logo are trademarks of Apple Inc.
- This project is not affiliated with, endorsed by, or sponsored by Apple Inc.

### Fair Use Rationale

This project exists for:
- Educational purposes (teaching OS design and implementation)
- Historical preservation (documenting 1980s-90s OS architecture)
- Interoperability research (understanding file formats and protocols)

**Original System 7 ROM and software remain property of Apple Inc.** This reimplementation does not include or require original Apple software to function.

## 🙏 Acknowledgments

- **Apple Computer, Inc.** for creating the original System 7
- **Inside Macintosh authors** for comprehensive documentation
- **Classic Mac preservation community** for keeping the platform alive
- **68k.news and Macintosh Garden** for resource archives and documentation

## 📊 Project Statistics

- **Lines of Code**: ~50,000+
- **Development Time**: Several months (ongoing)
- **Compilation Time**: ~3-5 seconds on modern hardware
- **Kernel Size**: ~860KB (kernel.elf)
- **ISO Size**: ~12.4MB (system71.iso)
- **Documented Findings**: 57 with full provenance
- **Error Reduction**: 94% (30 of 32 Window Manager errors resolved)

## 🔮 Future Direction

This project is in **active development** with no guaranteed timeline. Planned work includes:

**Short Term** (Proof of Concept Completion):
- Fix window dragging functionality
- Implement menu dropdown display
- Complete window update event pipeline
- Fix desktop icon drag artifacts

**Medium Term** (Core Toolbox):
- Complete Window Definition Procedure (WDEF) dispatch
- Implement standard controls (buttons, scrollbars, text fields)
- Add TextEdit for basic text input
- Implement functional dialog boxes

**Long Term** (Application Support):
- Resource Manager with .rsrc file support
- Process Manager for application launching
- Segment Loader for loading CODE resources
- Basic desk accessories (Calculator, Note Pad)

**Stretch Goals** (Full System):
- Sound Manager with sample playback
- Printing system (PostScript generation)
- AppleTalk networking stack
- Color QuickDraw with 8-bit palette support

---

**Status**: Experimental - Educational - In Development

**Last Updated**: September 2025

For questions, issues, or discussion, please use GitHub Issues.
