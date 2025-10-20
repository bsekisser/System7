# Finder Application Switcher Implementation Plan

## Overview
Implement the classic Mac OS application switcher (Command-Tab) that allows users to cycle through open applications with a visual window showing application icons and names.

## Current State Analysis

### Existing Infrastructure
✅ **Process Manager** (`ProcessManager.c`):
- Process tracking system exists
- Process queue and control blocks implemented
- MultiFinder support integrated
- Application process list available

✅ **Event System** (`EventManager/KeyboardEvents.c`):
- Keyboard event capture working
- Modifier key detection (Command, Shift, etc.)
- Event posting mechanism functional

✅ **Window Manager** (`WindowManager/`):
- Window creation and display
- Drawing and rendering
- Event routing

✅ **Icon System** (`Finder/Icon/`):
- Icon loading from resources
- Icon rendering (ARGB32 format)
- Icon caching

### Missing Components
❌ **Command-Tab Detection**: Not intercepted in KeyboardEvents
❌ **AppSwitcher Module**: No dedicated switcher implementation
❌ **Switcher UI**: No window/drawing for app switcher display
❌ **Visual Rendering**: No switcher list display logic
❌ **Focus Switching**: No mechanism to bring app to front after selection

## Implementation Architecture

### 1. AppSwitcher Core Module (`src/EventManager/AppSwitcher.c`)

**Data Structures**:
```c
typedef struct {
    ProcessSerialNumber psn;
    Str255 appName;
    IconFamily icon;
    Boolean isHidden;
} SwitchableApp;

typedef struct {
    SwitchableApp* apps;
    SInt16 appCount;
    SInt16 currentIndex;
    Boolean isActive;
    Point windowPos;
    Rect windowBounds;
    GrafPtr switcherPort;
} AppSwitcherState;
```

**Core Functions**:
- `AppSwitcher_Init()` - Initialize switcher state
- `AppSwitcher_ShowWindow()` - Display switcher UI
- `AppSwitcher_HideWindow()` - Hide switcher UI
- `AppSwitcher_CycleForward()` - Move to next app
- `AppSwitcher_CycleBackward()` - Move to previous app
- `AppSwitcher_SelectCurrent()` - Switch to selected app
- `AppSwitcher_Draw()` - Render switcher window
- `AppSwitcher_UpdateAppList()` - Refresh app list from ProcessManager

### 2. Keyboard Event Integration (`EventManager/KeyboardEvents.c`)

**Modifications**:
- Add Command-Tab detection in key event processing
- Hook into modifier key state tracking
- Detect Tab key press while Command is held
- Check for Shift modifier for reverse cycling

**Logic Flow**:
```
Tab key pressed?
  ├─ Command held?
  │  ├─ Shift held?
  │  │  └─ CycleBackward()
  │  └─ CycleForward()
  └─ Normal tab behavior
```

### 3. ProcessManager Integration

**Required Functions**:
- `ProcessManager_GetAppList()` - Return array of running apps
- `ProcessManager_GetFrontProcess()` - Return currently active app
- `ProcessManager_SetFrontProcess()` - Bring app to front

### 4. UI/Drawing Layer

**Switcher Window Design**:
- Centered modal window (approx 400x300 pixels)
- Application icon (32x32) + name for each app
- Current selection highlighted
- Semi-transparent background (fade effect)
- Close on Tab release

**Drawing Elements**:
- Window frame (gray box with black border)
- Icon rendering for each app
- Application name text
- Highlight box around current selection

## Implementation Steps

### Phase 1: Core Infrastructure (Hours 1-3)
1. Create `AppSwitcher.h` header with data structures
2. Create `AppSwitcher.c` with init/state management
3. Add ProcessManager functions to get app list
4. Set up switcher state globals

### Phase 2: Keyboard Event Handling (Hours 4-6)
1. Add Command-Tab detection in KeyboardEvents.c
2. Implement key event routing to AppSwitcher
3. Handle Tab press/release events
4. Test keyboard capture

### Phase 3: UI Window & Drawing (Hours 7-10)
1. Implement window creation/display
2. Create drawing functions for switcher UI
3. Render icons and app names
4. Implement highlight tracking
5. Handle window positioning (center screen)

### Phase 4: Application Switching (Hours 11-13)
1. Implement process focus switching logic
2. Hook ProcessManager_SetFrontProcess()
3. Test app bring-to-front functionality
4. Handle edge cases (hidden apps, disabled processes)

### Phase 5: Polish & Testing (Hours 14-15)
1. Add smooth animation/transitions
2. Test with multiple applications
3. Fix edge cases and crashes
4. Performance optimization

## File Changes Required

### New Files
- `src/EventManager/AppSwitcher.h`
- `src/EventManager/AppSwitcher.c`
- `include/EventManager/AppSwitcher.h`

### Modified Files
- `src/EventManager/KeyboardEvents.c` - Add Command-Tab detection
- `src/ProcessMgr/ProcessManager.c` - Add app list retrieval functions
- `src/EventManager/EventManagerInternal.h` - Add AppSwitcher declarations
- `Makefile` - Add new source files to build

## Data Flow

```
User presses Command-Tab
    ↓
KeyboardEvents.c detects modifier + Tab
    ↓
Calls AppSwitcher_ShowWindow()
    ↓
AppSwitcher fetches app list from ProcessManager
    ↓
Renders switcher UI window
    ↓
Cycles through apps on repeated Tab presses
    ↓
User releases Tab key
    ↓
AppSwitcher_SelectCurrent() brings selected app to front
    ↓
Hides switcher window
```

## Key Implementation Details

### 1. Command Key Detection
- Use existing modifier key state tracking in KeyboardEvents
- Check if Command key (macCmd) is held: `modifiers & cmdKey`
- Tab key scan code: 0x30

### 2. Process Switching
- Use `ProcessManager_GetFrontProcess()` to get current focus
- Use `ProcessManager_SetFrontProcess()` to switch
- Send activation events to applications
- Update menu bar to reflect new front process

### 3. Visual Feedback
- Draw semi-transparent overlay window
- Center on screen
- Show icon + name for each app
- Highlight current selection with invert rect or color
- Support both horizontal and vertical layouts

### 4. Event Ordering
- Switcher activation: keyDown event
- Cycle forward/backward: repeated keyDown events
- Selection: keyUp event (Tab released)
- Hide switcher: after selection or Tab release

## Testing Checklist

- [ ] Command-Tab opens switcher window
- [ ] Window displays all running applications
- [ ] Tab cycles to next application
- [ ] Shift-Tab cycles to previous application
- [ ] Releasing Tab switches to selected app
- [ ] Switching brings app's windows to front
- [ ] Menu bar updates to reflect new front app
- [ ] Works with 2+ applications
- [ ] No crashes with single app
- [ ] Hidden applications handled correctly
- [ ] System process not included in list
- [ ] Visual appearance matches System 7 style

## Complexity Estimation

**Total Effort: 15-20 hours**

- Core infrastructure: 3 hours
- Keyboard integration: 3 hours
- UI/Drawing: 4 hours
- Process switching: 3 hours
- Testing/polish: 2-3 hours

## Dependencies Met
- ✅ ProcessManager (exists)
- ✅ EventManager (exists)
- ✅ WindowManager (exists)
- ✅ QuickDraw (exists)
- ✅ Icon system (exists)
- ✅ Menu system (exists)

## Success Criteria

1. Users can switch between applications using Command-Tab
2. Visual switcher window displays all running apps
3. App brings to front after selection
4. No crashes or memory leaks
5. Works with existing test applications
6. Menu bar updates correctly
7. Performance acceptable (no lag)

## Future Enhancements

- Animated transitions
- Sound effects (beep on cycle)
- Application filtering (show/hide system processes)
- Customizable layout (horizontal vs vertical)
- Window preview thumbnails
- Keyboard configuration options
