# malloc/free/calloc/realloc Audit Report

**Generated:** 2025-10-19
**Purpose:** Systematic code audit to identify and prioritize malloc/free → NewPtr/DisposePtr conversions for heap corruption bug fixes

---

## Executive Summary

- **Total Files with malloc/free/calloc/realloc:** 89 files
- **Total malloc calls:** 160
- **Total free calls:** 377
- **Total calloc calls:** 61
- **Total realloc calls:** 19
- **GRAND TOTAL:** 617 calls requiring audit/conversion

---

## Priority Recommendations

The subsystems are prioritized by total call count (highest risk first):

| Priority | Subsystem | Total Calls | Files | malloc | free | calloc | realloc |
|----------|-----------|-------------|-------|--------|------|--------|---------|
| 1 | **FileMgr** | 88 | 12 | 18 | 60 | 10 | 0 |
| 2 | **FontManager** | 80 | 3 | 11 | 57 | 9 | 3 |
| 3 | **SpeechManager** | 69 | 7 | 24 | 43 | 0 | 2 |
| 4 | **AppleEventManager** | 50 | 7 | 16 | 31 | 2 | 1 |
| 5 | **DeskManager** | 48 | 11 | 16 | 26 | 6 | 0 |
| 6 | **SoundManager** | 44 | 5 | 1 | 29 | 14 | 0 |
| 7 | **DialogManager** | 39 | 6 | 17 | 20 | 0 | 2 |
| 8 | **ResourceMgr** | 29 | 2 | 3 | 22 | 3 | 1 |
| 9 | **CoreSystem** | 21 | 5 | 1 | 11 | 9 | 0 |
| 10 | **DeviceManager** | 19 | 4 | 7 | 11 | 0 | 1 |
| 11 | **Finder** | 16 | 2 | 7 | 6 | 1 | 2 |
| 12 | **MemoryMgr** | 15 | 3 | 5 | 6 | 2 | 2 |
| 13 | **TextEdit** | 15 | 3 | 6 | 9 | 0 | 0 |
| 14 | **EventManager** | 12 | 4 | 3 | 8 | 1 | 0 |
| 15 | **ExtensionManager** | 12 | 6 | 6 | 6 | 0 | 0 |
| 16 | **ScrapManager** | 12 | 3 | 4 | 8 | 0 | 0 |
| 17 | **MenuManager** | 11 | 1 | 2 | 9 | 0 | 0 |
| 18 | **StandardFile** | 9 | 2 | 4 | 4 | 0 | 1 |
| 19 | **WindowManager** | 8 | 4 | 1 | 3 | 4 | 0 |
| 20 | **Apps** | 6 | 1 | 2 | 4 | 0 | 0 |
| 21 | **ControlManager** | 5 | 2 | 3 | 2 | 0 | 0 |
| 22 | **Chooser** | 5 | 1 | 3 | 2 | 0 | 0 |
| 23 | **QuickDraw** | 4 | 1 | 0 | 0 | 0 | 4 |

---

## Detailed Breakdown by Subsystem

### 1. FileMgr (88 calls across 12 files) - HIGHEST PRIORITY

**Files requiring attention:**

1. `/home/k/iteration2/src/FS/hfs_diskio.c` - 14 calls (5 malloc, 9 free)
2. `/home/k/iteration2/src/FileMgr/volume_manager.c` - 9 calls (2 malloc, 7 free)
3. `/home/k/iteration2/src/FS/hfs_btree.c` - 9 calls (3 malloc, 6 free)
4. `/home/k/iteration2/src/FS/hfs_volume.c` - 9 calls (1 malloc, 8 free)
5. `/home/k/iteration2/src/HFS_Allocation.c` - 8 calls (3 calloc, 5 free)
6. `/home/k/iteration2/src/FileManager.c` - 7 calls (2 calloc, 5 free)
7. `/home/k/iteration2/src/FileMgr/btree_services.c` - 7 calls (2 malloc, 5 free)
8. `/home/k/iteration2/src/FS/vfs.c` - 7 calls (3 malloc, 4 free)
9. `/home/k/iteration2/src/HFS_Catalog.c` - 5 calls (2 calloc, 3 free)
10. `/home/k/iteration2/src/FS/hfs_file.c` - 5 calls (2 malloc, 3 free)
11. `/home/k/iteration2/src/HFS_Volume.c` - 4 calls (1 calloc, 3 free)
12. `/home/k/iteration2/src/FileManagerStubs.c` - 4 calls (2 calloc, 2 free)

**Risk Assessment:** HIGH - File system operations are critical and memory corruption here can cause data loss.

---

### 2. FontManager (80 calls across 3 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/FontManager/ModernFontSupport.c` - 46 calls (3 malloc, 36 free, 7 calloc)
2. `/home/k/iteration2/src/FontManager/WebFontSupport.c` - 31 calls (7 malloc, 19 free, 2 calloc, 3 realloc)
3. `/home/k/iteration2/src/FontManager/ModernFontDetection.c` - 3 calls (1 malloc, 2 free)

**Risk Assessment:** HIGH - Font operations are frequent and memory leaks here impact system stability.

---

### 3. SpeechManager (69 calls across 7 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/SpeechManager/TextToSpeech.c` - 17 calls (10 malloc, 7 free)
2. `/home/k/iteration2/src/SpeechManager/VoiceResources.c` - 16 calls (4 malloc, 12 free)
3. `/home/k/iteration2/src/SpeechManager/PronunciationEngine.c` - 15 calls (4 malloc, 10 free, 1 realloc)
4. `/home/k/iteration2/src/SpeechManager/SpeechSynthesis.c` - 8 calls (2 malloc, 6 free)
5. `/home/k/iteration2/src/SpeechManager/SpeechOutput.c` - 6 calls (3 malloc, 3 free)
6. `/home/k/iteration2/src/SpeechManager/SpeechChannels.c` - 5 calls (1 malloc, 3 free, 1 realloc)
7. `/home/k/iteration2/src/SpeechManager/VoiceManager.c` - 2 calls (2 free)

**Risk Assessment:** MEDIUM-HIGH - Speech processing involves buffers that could be corrupted.

---

### 4. AppleEventManager (50 calls across 7 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/AppleEventManager/AppleEventManagerCore.c` - 12 calls (3 malloc, 8 free, 1 realloc)
2. `/home/k/iteration2/src/AppleEventManager/EventDescriptors.c` - 11 calls (3 malloc, 8 free)
3. `/home/k/iteration2/src/AppleEventManager/EventRecording.c` - 8 calls (3 malloc, 3 free, 2 calloc)
4. `/home/k/iteration2/src/AppleEventManager/EventHandlers.c` - 6 calls (3 malloc, 3 free)
5. `/home/k/iteration2/src/AppleEventManager/AppleEventExample.c` - 6 calls (1 malloc, 5 free)
6. `/home/k/iteration2/src/AppleEventManager/EventCoercion.c` - 4 calls (2 malloc, 2 free)
7. `/home/k/iteration2/src/AppleEventManager/AppleEventDispatch.c` - 3 calls (1 malloc, 2 free)

**Risk Assessment:** MEDIUM - AppleEvents are critical for inter-application communication.

---

### 5. DeskManager (48 calls across 11 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/DeskManager/BuiltinDAs.c` - 12 calls (4 malloc, 8 free)
2. `/home/k/iteration2/src/DeskManager/DALoader.c` - 10 calls (2 malloc, 6 free, 2 calloc)
3. `/home/k/iteration2/src/DeskManager/Chooser.c` - 6 calls (4 malloc, 2 free)
4. `/home/k/iteration2/src/DeskManager/DAWindows.c` - 4 calls (1 malloc, 2 free, 1 calloc)
5. `/home/k/iteration2/src/DeskManager/SystemMenu.c` - 4 calls (1 malloc, 2 free, 1 calloc)
6. `/home/k/iteration2/src/DeskManager/Notepad.c` - 3 calls (1 calloc, 2 free)
7. `/home/k/iteration2/src/DeskManager/AlarmClock.c` - 2 calls (1 malloc, 1 free)
8. `/home/k/iteration2/src/DeskManager/DAPreferences.c` - 2 calls (2 malloc)
9. `/home/k/iteration2/src/DeskManager/KeyCaps.c` - 2 calls (1 malloc, 1 free)
10. `/home/k/iteration2/src/DeskManager/DeskManagerCore.c` - 2 calls (1 calloc, 1 free)
11. `/home/k/iteration2/src/DeskManager/Calculator.c` - 1 call (1 free)

**Risk Assessment:** MEDIUM - DA operations are user-facing and memory leaks are visible.

---

### 6. SoundManager (44 calls across 5 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/SoundManager/SoundMixing.c` - 24 calls (7 calloc, 17 free)
2. `/home/k/iteration2/src/SoundManager/SoundManagerCore.c` - 7 calls (2 calloc, 5 free)
3. `/home/k/iteration2/src/SoundManager/SoundHardware.c` - 6 calls (3 calloc, 3 free)
4. `/home/k/iteration2/src/SoundManager/SoundMIDI.c` - 5 calls (1 malloc, 4 free)
5. `/home/k/iteration2/src/SoundManager/SoundResources.c` - 2 calls (2 calloc)

**Risk Assessment:** MEDIUM - Sound buffers need careful management.

---

### 7. DialogManager (39 calls across 6 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/DialogManager/DialogResources.c` - 14 calls (8 malloc, 6 free)
2. `/home/k/iteration2/src/DialogManager/DialogResourceParser.c` - 7 calls (4 malloc, 3 free)
3. `/home/k/iteration2/src/DialogManager/DialogItems.c` - 6 calls (2 realloc, 4 free)
4. `/home/k/iteration2/src/DialogManager/dialog_manager_dispatch.c` - 6 calls (3 malloc, 3 free)
5. `/home/k/iteration2/src/DialogManager/AlertDialogs.c` - 4 calls (1 malloc, 3 free)
6. `/home/k/iteration2/src/DialogManager/dialog_manager_core.c` - 2 calls (1 malloc, 1 free)

**Risk Assessment:** MEDIUM - Dialogs are critical UI components.

---

### 8. ResourceMgr (29 calls across 2 files)

**Files requiring attention:**

1. `/home/k/iteration2/src/ResourceManager.c` - 22 calls (1 malloc, 17 free, 3 calloc, 1 realloc)
2. `/home/k/iteration2/src/ResourceMgr/resource_manager.c` - 7 calls (2 malloc, 5 free)

**Risk Assessment:** HIGH - Resource manager is fundamental to system operation.

---

## Special Cases Requiring Attention

### realloc() Usage (19 total calls)

**Note:** realloc() is particularly problematic in bare-metal environments and has been noted as broken in some kernel code.

1. `/home/k/iteration2/src/QuickDraw/Regions.c` - 4 realloc (COMMENTED AS BROKEN)
2. `/home/k/iteration2/src/FontManager/WebFontSupport.c` - 3 realloc
3. `/home/k/iteration2/src/Finder/folder_window.c` - 2 realloc
4. `/home/k/iteration2/src/DialogManager/DialogItems.c` - 2 realloc
5. `/home/k/iteration2/src/MemoryMgr/MemoryManager.c` - 1 realloc
6. `/home/k/iteration2/src/SpeechManager/PronunciationEngine.c` - 1 realloc
7. `/home/k/iteration2/src/SpeechManager/SpeechChannels.c` - 1 realloc
8. `/home/k/iteration2/src/AppleEventManager/AppleEventManagerCore.c` - 1 realloc
9. `/home/k/iteration2/src/ResourceManager.c` - 1 realloc
10. `/home/k/iteration2/src/DeviceManager/UnitTable.c` - 1 realloc
11. `/home/k/iteration2/src/StandardFile/StandardFileHAL_Shims.c` - 1 realloc

**Action Required:** All realloc() calls should be replaced with NewPtr/DisposePtr pattern (allocate new, copy, dispose old).

---

## Memory Imbalance Analysis

Files with significantly more free() than malloc() (potential external allocations):

1. `/home/k/iteration2/src/FontManager/ModernFontSupport.c` - 36 free vs 3 malloc (33 diff)
2. `/home/k/iteration2/src/FontManager/WebFontSupport.c` - 19 free vs 7 malloc (12 diff)
3. `/home/k/iteration2/src/ResourceManager.c` - 17 free vs 1 malloc (16 diff)

Files with significantly more malloc() than free() (potential memory leaks):

1. `/home/k/iteration2/src/SpeechManager/TextToSpeech.c` - 10 malloc vs 7 free (3 diff)
2. `/home/k/iteration2/src/DeskManager/Chooser.c` - 4 malloc vs 2 free (2 diff)
3. `/home/k/iteration2/src/DAPreferences.c` - 2 malloc vs 0 free (2 diff - LEAK!)

---

## Conversion Strategy Recommendations

### Phase 1: Critical Infrastructure (Week 1-2)
1. **FileMgr** - All HFS and file system code
2. **ResourceMgr** - Resource loading/management
3. **MemoryMgr** - Memory manager itself

### Phase 2: High-Volume Subsystems (Week 3-4)
4. **FontManager** - Heavy malloc/free usage
5. **SpeechManager** - Complex buffer management
6. **SoundManager** - Audio buffer management

### Phase 3: Application Layer (Week 5-6)
7. **DialogManager** - UI components
8. **WindowManager** - Already partially converted
9. **EventManager** - Event processing
10. **AppleEventManager** - Inter-app communication

### Phase 4: Extensions & Utilities (Week 7-8)
11. **DeskManager** - Desk accessories
12. **ExtensionManager** - Extension loaders
13. **DeviceManager** - Device drivers
14. **StandardFile** - File dialogs

### Phase 5: Lower Priority (Week 9-10)
15. **ScrapManager** - Clipboard operations
16. **TextEdit** - Text editing
17. **ControlManager** - UI controls
18. **MenuManager** - Menu operations
19. **Finder** - Finder operations
20. **Apps** - Application code

---

## Files to Audit Immediately (Top 20 by call count)

1. `/home/k/iteration2/src/FontManager/ModernFontSupport.c` - 46 calls
2. `/home/k/iteration2/src/FontManager/WebFontSupport.c` - 31 calls
3. `/home/k/iteration2/src/SoundManager/SoundMixing.c` - 24 calls
4. `/home/k/iteration2/src/ResourceManager.c` - 22 calls
5. `/home/k/iteration2/src/SpeechManager/TextToSpeech.c` - 17 calls
6. `/home/k/iteration2/src/SpeechManager/VoiceResources.c` - 16 calls
7. `/home/k/iteration2/src/SpeechManager/PronunciationEngine.c` - 15 calls
8. `/home/k/iteration2/src/Finder/folder_window.c` - 15 calls
9. `/home/k/iteration2/src/FS/hfs_diskio.c` - 14 calls
10. `/home/k/iteration2/src/DialogManager/DialogResources.c` - 14 calls
11. `/home/k/iteration2/src/DeskManager/BuiltinDAs.c` - 12 calls
12. `/home/k/iteration2/src/AppleEventManager/AppleEventManagerCore.c` - 12 calls
13. `/home/k/iteration2/src/DeviceManager/UnitTable.c` - 12 calls
14. `/home/k/iteration2/src/TextEdit/textedit_core.c` - 11 calls
15. `/home/k/iteration2/src/MenuCommands.c` - 11 calls
16. `/home/k/iteration2/src/AppleEventManager/EventDescriptors.c` - 11 calls
17. `/home/k/iteration2/src/DeskManager/DALoader.c` - 10 calls
18. `/home/k/iteration2/src/MemoryMgr/MemoryManager.c` - 9 calls
19. `/home/k/iteration2/src/FileMgr/volume_manager.c` - 9 calls
20. `/home/k/iteration2/src/FS/hfs_btree.c` - 9 calls

---

## Notes

- **Excluded:** Test files, third-party code, documentation (.md files)
- **Header Files:** Include file `/home/k/iteration2/include/MemoryMgr/MemoryManager.h` contains 4 calls (likely inline functions or macros)
- **QuickDraw Regions:** Contains comments indicating realloc() is broken in bare-metal kernel
- **Recent Fixes:** WindowDisplay.c and TextInput.c have recently been fixed (recent commits show NewPtr/DisposePtr conversions)

---

## Recommended Next Steps

1. Start with **FileMgr subsystem** (highest priority, fundamental to system)
2. Focus on files with **realloc()** usage (known problematic)
3. Review files with **memory imbalances** (potential leaks)
4. Create unit tests for each converted file
5. Test incrementally after each subsystem conversion

---

## Data Files

- **Full Analysis:** `/home/k/iteration2/malloc_analysis_full.txt`
- **CSV Export:** `/home/k/iteration2/malloc_audit_summary.csv`
- **This Report:** `/home/k/iteration2/MALLOC_AUDIT_REPORT.md`
