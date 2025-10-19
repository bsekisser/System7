# malloc/free → NewPtr/DisposePtr Conversion Priority List

**Last Updated:** 2025-10-19
**Total Remaining:** 617 malloc/free/calloc/realloc calls across 89 files

---

## CRITICAL PRIORITY - Fix Immediately

These files have the highest risk due to critical subsystems or high call counts:

### FileMgr Subsystem (88 calls total)
- [ ] `/home/k/iteration2/src/FS/hfs_diskio.c` (14 calls: 5 malloc, 9 free)
- [ ] `/home/k/iteration2/src/FileMgr/volume_manager.c` (9 calls: 2 malloc, 7 free)
- [ ] `/home/k/iteration2/src/FS/hfs_btree.c` (9 calls: 3 malloc, 6 free)
- [ ] `/home/k/iteration2/src/FS/hfs_volume.c` (9 calls: 1 malloc, 8 free)
- [ ] `/home/k/iteration2/src/HFS_Allocation.c` (8 calls: 0 malloc, 5 free, 3 calloc)
- [ ] `/home/k/iteration2/src/FileManager.c` (7 calls: 0 malloc, 5 free, 2 calloc)
- [ ] `/home/k/iteration2/src/FileMgr/btree_services.c` (7 calls: 2 malloc, 5 free)
- [ ] `/home/k/iteration2/src/FS/vfs.c` (7 calls: 3 malloc, 4 free)
- [ ] `/home/k/iteration2/src/HFS_Catalog.c` (5 calls: 0 malloc, 3 free, 2 calloc)
- [ ] `/home/k/iteration2/src/FS/hfs_file.c` (5 calls: 2 malloc, 3 free)
- [ ] `/home/k/iteration2/src/HFS_Volume.c` (4 calls: 0 malloc, 3 free, 1 calloc)
- [ ] `/home/k/iteration2/src/FileManagerStubs.c` (4 calls: 0 malloc, 2 free, 2 calloc)

### ResourceMgr Subsystem (29 calls total)
- [ ] `/home/k/iteration2/src/ResourceManager.c` (22 calls: 1 malloc, 17 free, 3 calloc, 1 realloc)
- [ ] `/home/k/iteration2/src/ResourceMgr/resource_manager.c` (7 calls: 2 malloc, 5 free)

---

## HIGH PRIORITY - Week 1-2

### FontManager Subsystem (80 calls total) - HEAVY USAGE
- [ ] `/home/k/iteration2/src/FontManager/ModernFontSupport.c` (46 calls: 3 malloc, 36 free, 7 calloc) ⚠️ MOST CALLS
- [ ] `/home/k/iteration2/src/FontManager/WebFontSupport.c` (31 calls: 7 malloc, 19 free, 2 calloc, 3 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/FontManager/ModernFontDetection.c` (3 calls: 1 malloc, 2 free)

### SpeechManager Subsystem (69 calls total)
- [ ] `/home/k/iteration2/src/SpeechManager/TextToSpeech.c` (17 calls: 10 malloc, 7 free)
- [ ] `/home/k/iteration2/src/SpeechManager/VoiceResources.c` (16 calls: 4 malloc, 12 free)
- [ ] `/home/k/iteration2/src/SpeechManager/PronunciationEngine.c` (15 calls: 4 malloc, 10 free, 1 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/SpeechManager/SpeechSynthesis.c` (8 calls: 2 malloc, 6 free)
- [ ] `/home/k/iteration2/src/SpeechManager/SpeechOutput.c` (6 calls: 3 malloc, 3 free)
- [ ] `/home/k/iteration2/src/SpeechManager/SpeechChannels.c` (5 calls: 1 malloc, 3 free, 1 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/SpeechManager/VoiceManager.c` (2 calls: 0 malloc, 2 free)

---

## MEDIUM-HIGH PRIORITY - Week 3-4

### SoundManager Subsystem (44 calls total)
- [ ] `/home/k/iteration2/src/SoundManager/SoundMixing.c` (24 calls: 0 malloc, 17 free, 7 calloc)
- [ ] `/home/k/iteration2/src/SoundManager/SoundManagerCore.c` (7 calls: 0 malloc, 5 free, 2 calloc)
- [ ] `/home/k/iteration2/src/SoundManager/SoundHardware.c` (6 calls: 0 malloc, 3 free, 3 calloc)
- [ ] `/home/k/iteration2/src/SoundManager/SoundMIDI.c` (5 calls: 1 malloc, 4 free)
- [ ] `/home/k/iteration2/src/SoundManager/SoundResources.c` (2 calls: 0 malloc, 0 free, 2 calloc)

### AppleEventManager Subsystem (50 calls total)
- [ ] `/home/k/iteration2/src/AppleEventManager/AppleEventManagerCore.c` (12 calls: 3 malloc, 8 free, 0 calloc, 1 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/AppleEventManager/EventDescriptors.c` (11 calls: 3 malloc, 8 free)
- [ ] `/home/k/iteration2/src/AppleEventManager/EventRecording.c` (8 calls: 3 malloc, 3 free, 2 calloc)
- [ ] `/home/k/iteration2/src/AppleEventManager/EventHandlers.c` (6 calls: 3 malloc, 3 free)
- [ ] `/home/k/iteration2/src/AppleEventManager/AppleEventExample.c` (6 calls: 1 malloc, 5 free)
- [ ] `/home/k/iteration2/src/AppleEventManager/EventCoercion.c` (4 calls: 2 malloc, 2 free)
- [ ] `/home/k/iteration2/src/AppleEventManager/AppleEventDispatch.c` (3 calls: 1 malloc, 2 free)

### DialogManager Subsystem (39 calls total)
- [ ] `/home/k/iteration2/src/DialogManager/DialogResources.c` (14 calls: 8 malloc, 6 free)
- [ ] `/home/k/iteration2/src/DialogManager/DialogResourceParser.c` (7 calls: 4 malloc, 3 free)
- [ ] `/home/k/iteration2/src/DialogManager/DialogItems.c` (6 calls: 0 malloc, 4 free, 0 calloc, 2 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/DialogManager/dialog_manager_dispatch.c` (6 calls: 3 malloc, 3 free)
- [ ] `/home/k/iteration2/src/DialogManager/AlertDialogs.c` (4 calls: 1 malloc, 3 free)
- [ ] `/home/k/iteration2/src/DialogManager/dialog_manager_core.c` (2 calls: 1 malloc, 1 free)

---

## MEDIUM PRIORITY - Week 5-6

### DeskManager Subsystem (48 calls total)
- [ ] `/home/k/iteration2/src/DeskManager/BuiltinDAs.c` (12 calls: 4 malloc, 8 free)
- [ ] `/home/k/iteration2/src/DeskManager/DALoader.c` (10 calls: 2 malloc, 6 free, 2 calloc)
- [ ] `/home/k/iteration2/src/DeskManager/Chooser.c` (6 calls: 4 malloc, 2 free)
- [ ] `/home/k/iteration2/src/DeskManager/DAWindows.c` (4 calls: 1 malloc, 2 free, 1 calloc)
- [ ] `/home/k/iteration2/src/DeskManager/SystemMenu.c` (4 calls: 1 malloc, 2 free, 1 calloc)
- [ ] `/home/k/iteration2/src/DeskManager/Notepad.c` (3 calls: 0 malloc, 2 free, 1 calloc)
- [ ] `/home/k/iteration2/src/DeskManager/AlarmClock.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/DeskManager/DAPreferences.c` (2 calls: 2 malloc, 0 free) ⚠️ MEMORY LEAK!
- [ ] `/home/k/iteration2/src/DeskManager/KeyCaps.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/DeskManager/DeskManagerCore.c` (2 calls: 0 malloc, 1 free, 1 calloc)
- [ ] `/home/k/iteration2/src/DeskManager/Calculator.c` (1 call: 0 malloc, 1 free)

### DeviceManager Subsystem (19 calls total)
- [ ] `/home/k/iteration2/src/DeviceManager/UnitTable.c` (12 calls: 4 malloc, 7 free, 0 calloc, 1 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/DeviceManager/DeviceInterrupts.c` (3 calls: 1 malloc, 2 free)
- [ ] `/home/k/iteration2/src/DeviceManager/DriverDispatch.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/DeviceManager/DeviceIO.c` (2 calls: 1 malloc, 1 free)

### CoreSystem (21 calls total)
- [ ] `/home/k/iteration2/src/ExpandMem.c` (7 calls: 0 malloc, 4 free, 3 calloc)
- [ ] `/home/k/iteration2/src/SystemInit.c` (6 calls: 0 malloc, 4 free, 2 calloc)
- [ ] `/home/k/iteration2/src/SCSIManager.c` (4 calls: 0 malloc, 2 free, 2 calloc)
- [ ] `/home/k/iteration2/src/TrapDispatcher.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ResourceDecompression.c` (2 calls: 0 malloc, 0 free, 2 calloc)

---

## LOWER PRIORITY - Week 7-8

### Finder Subsystem (16 calls total)
- [ ] `/home/k/iteration2/src/Finder/folder_window.c` (15 calls: 7 malloc, 6 free, 0 calloc, 2 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/Finder/desktop_manager.c` (1 call: 0 malloc, 0 free, 1 calloc)

### MemoryMgr Subsystem (15 calls total) - SPECIAL CARE NEEDED
- [ ] `/home/k/iteration2/src/MemoryMgr/MemoryManager.c` (9 calls: 4 malloc, 3 free, 1 calloc, 1 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/include/MemoryMgr/MemoryManager.h` (4 calls: 1 malloc, 1 free, 1 calloc, 1 realloc) ⚠️ HAS REALLOC
- [ ] `/home/k/iteration2/src/MemoryMgr/heap_compaction.c` (2 calls: 0 malloc, 2 free)

### TextEdit Subsystem (15 calls total)
- [ ] `/home/k/iteration2/src/TextEdit/textedit_core.c` (11 calls: 4 malloc, 7 free)
- [ ] `/home/k/iteration2/src/TextEdit/TextInput.c` (2 calls: 1 malloc, 1 free) ✅ Recently fixed
- [ ] `/home/k/iteration2/src/TextEdit/TextUndo.c` (2 calls: 1 malloc, 1 free)

### EventManager Subsystem (12 calls total)
- [ ] `/home/k/iteration2/src/EventManager/SystemEvents.c` (7 calls: 2 malloc, 5 free)
- [ ] `/home/k/iteration2/src/EventManager/MouseEvents.c` (3 calls: 1 malloc, 2 free)
- [ ] `/home/k/iteration2/src/EventManager/EventManagerCore.c` (1 call: 0 malloc, 0 free, 1 calloc)
- [ ] `/home/k/iteration2/src/EventManager/KeyboardEvents.c` (1 call: 0 malloc, 1 free)

### ExtensionManager Subsystem (12 calls total)
- [ ] `/home/k/iteration2/src/ExtensionManager/FKEYLoader.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ExtensionManager/ControlPanelManager.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ExtensionManager/CDEFLoader.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ExtensionManager/DRVRLoader.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ExtensionManager/DefLoader.c` (2 calls: 1 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ExtensionManager/ExtensionManagerCore.c` (2 calls: 1 malloc, 1 free)

### ScrapManager Subsystem (12 calls total)
- [ ] `/home/k/iteration2/src/ScrapManager/InterAppScrap.c` (6 calls: 2 malloc, 4 free)
- [ ] `/home/k/iteration2/src/ScrapManager/ModernClipboard.c` (4 calls: 1 malloc, 3 free)
- [ ] `/home/k/iteration2/src/ScrapManager/ScrapMemory.c` (2 calls: 1 malloc, 1 free)

### MenuManager Subsystem (11 calls total)
- [ ] `/home/k/iteration2/src/MenuCommands.c` (11 calls: 2 malloc, 9 free)

### StandardFile Subsystem (9 calls total)
- [ ] `/home/k/iteration2/src/StandardFile/StandardFile.c` (7 calls: 3 malloc, 4 free)
- [ ] `/home/k/iteration2/src/StandardFile/StandardFileHAL_Shims.c` (2 calls: 1 malloc, 0 free, 0 calloc, 1 realloc) ⚠️ HAS REALLOC

### WindowManager Subsystem (8 calls total)
- [ ] `/home/k/iteration2/src/WindowManager/WindowManagerCore.c` (4 calls: 0 malloc, 1 free, 3 calloc)
- [ ] `/home/k/iteration2/src/WindowManager/WindowDisplay.c` (2 calls: 1 malloc, 1 free) ✅ Recently fixed
- [ ] `/home/k/iteration2/src/WindowManager/WindowEvents.c` (1 call: 0 malloc, 1 free)
- [ ] `/home/k/iteration2/src/WindowManager/WindowResizing.c` (1 call: 0 malloc, 0 free, 1 calloc)

---

## LOW PRIORITY - Week 9+

### Apps (6 calls total)
- [ ] `/home/k/iteration2/src/Apps/SimpleText/STFileIO.c` (6 calls: 2 malloc, 4 free)

### ControlManager Subsystem (5 calls total)
- [ ] `/home/k/iteration2/src/ControlManager/control_manager_sys7.c` (3 calls: 2 malloc, 1 free)
- [ ] `/home/k/iteration2/src/ControlManager/PlatformControls.c` (2 calls: 1 malloc, 1 free)

### Chooser (5 calls total)
- [ ] `/home/k/iteration2/src/Chooser/chooser_desk_accessory.c` (5 calls: 3 malloc, 2 free)

### QuickDraw Subsystem (4 calls total)
- [ ] `/home/k/iteration2/src/QuickDraw/Regions.c` (4 calls: 0 malloc, 0 free, 0 calloc, 4 realloc) ⚠️ ALL REALLOC - COMMENTED AS BROKEN!

---

## Special Attention Items

### Files with realloc() (BROKEN IN BARE-METAL KERNEL)
1. `/home/k/iteration2/src/QuickDraw/Regions.c` (4 realloc) - **ALREADY COMMENTED AS BROKEN**
2. `/home/k/iteration2/src/FontManager/WebFontSupport.c` (3 realloc)
3. `/home/k/iteration2/src/Finder/folder_window.c` (2 realloc)
4. `/home/k/iteration2/src/DialogManager/DialogItems.c` (2 realloc)
5. Multiple files with 1 realloc each (see full report)

### Files with Memory Leaks (malloc > free)
1. `/home/k/iteration2/src/DeskManager/DAPreferences.c` - 2 malloc, 0 free ⚠️
2. `/home/k/iteration2/src/SpeechManager/TextToSpeech.c` - 10 malloc, 7 free
3. `/home/k/iteration2/src/DeskManager/Chooser.c` - 4 malloc, 2 free

### Recently Fixed (Reference for conversion patterns)
- `/home/k/iteration2/src/WindowManager/WindowDisplay.c` - Menu background buffer fix
- `/home/k/iteration2/src/TextEdit/TextInput.c` - Unlocked handle dereference fix

---

## Conversion Pattern Reference

Based on recent fixes, the conversion pattern is:

```c
// OLD (malloc/free):
void* buffer = malloc(size);
if (!buffer) return memFullErr;
// ... use buffer ...
free(buffer);

// NEW (NewPtr/DisposePtr):
Ptr buffer = NewPtr(size);
if (!buffer) return MemError();
// ... use buffer ...
DisposePtr(buffer);
```

For realloc(), use NewPtr/BlockMove/DisposePtr:
```c
// OLD:
newPtr = realloc(oldPtr, newSize);

// NEW:
Ptr newPtr = NewPtr(newSize);
if (newPtr && oldPtr) {
    BlockMove(oldPtr, newPtr, oldSize);
    DisposePtr(oldPtr);
}
```

---

## Progress Tracking

- **Total Files:** 89
- **Completed:** 2 (WindowDisplay.c, TextInput.c)
- **Remaining:** 87
- **Total Calls Remaining:** 617

Track progress by checking off items as they are converted and tested.
