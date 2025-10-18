# System 7.1 Portable - Subsystem Priority Matrix

## Visual Priority Map

```
EFFORT (Hours)
      ↑
 120+ │         AppleTalk ★★★
      │         FileSharing ★★★
 100  │
      │
 80   │         ExtensionMgr ★★★
      │
 60   │         PrintMgr ★★★
      │         ColorQD ★★
 40   │
      │
 20   │         ResourceMgr ★★    SpeechMgr ★
      │
      │  FontMgr ★★★    SoundMgr ★
 10   │  ListMgr ★★★    TimeManager ★★
      │  ControlMgr ★★
      │
  0   └─────────────────────────────────────────→
      0%  20%  40%  60%  80%  100%  COMPLETION
      
★ = Impact (1-3 stars)
★★ = High Impact (2-3 stars)  
★★★ = Critical Impact (3 stars)
```

---

## Priority Matrix by Value/Effort

### QUADRANT 1: HIGH VALUE, LOW EFFORT (DO FIRST!)
```
┌─────────────────────────────────────┐
│ FontManager (6-8h)          ★★★    │
│ ListManager (2-3h)          ★★★    │
│ ControlManager (4-5h)       ★★     │
│ TimeManager (3-4h, opt)     ★★     │
│                                    │
│ → Phase 1 effort: 12-16 hours      │
│ → ROI: 95% UI complete             │
└─────────────────────────────────────┘
```

### QUADRANT 2: HIGH VALUE, HIGH EFFORT (DO SECOND)
```
┌─────────────────────────────────────┐
│ Print Manager (40-60h)      ★★★    │
│ AppleTalk Stack (80-120h)   ★★★    │
│ Extension Manager (50-80h)  ★★     │
│                                    │
│ → Strategic importance              │
│ → Foundation for other features    │
└─────────────────────────────────────┘
```

### QUADRANT 3: LOW VALUE, LOW EFFORT (FILL-INS)
```
┌─────────────────────────────────────┐
│ ResourceManager gaps (20-30h) ★     │
│ SoundManager gaps (10-15h)    ★     │
│                                    │
│ → Nice polish, not critical        │
└─────────────────────────────────────┘
```

### QUADRANT 4: LOW VALUE, HIGH EFFORT (SKIP)
```
┌─────────────────────────────────────┐
│ Script Manager (40-60h)     ★       │
│ Speech Manager (20-30h)     ★       │
│                                    │
│ → Rarely needed in practice        │
└─────────────────────────────────────┘
```

---

## Completion Status Dashboard

```
NEAR-COMPLETE (84-87%, 2-8 hours to finish)
╔════════════════════════════════════════╗
║ ████████████████████████░ 87% ListMgr ║
║ ████████████████████████░ 84% FontMgr ║
║ ████████████████████████░ 84% CtrlMgr ║
║ ███████████████████████░░ 75% TimeMgr ║
╚════════════════════════════════════════╝

PARTIALLY COMPLETE (50-70%, 10-30 hours to finish)
╔════════════════════════════════════════╗
║ ██████████████████████░░░░ 65% ResrcMgr
║ █████████████████░░░░░░░░░░ 50% WinMgr  
║ ████████████░░░░░░░░░░░░░░░ 40% MenuMgr 
║ ███████░░░░░░░░░░░░░░░░░░░░ 30% SpeechMgr
║ ████████████░░░░░░░░░░░░░░░ 40% Alias  
║ ███░░░░░░░░░░░░░░░░░░░░░░░░ 15% ColorQD 
╚════════════════════════════════════════╝

SIGNIFICANTLY INCOMPLETE (0-20%)
╔════════════════════════════════════════╗
║ ░░░░░░░░░░░░░░░░░░░░░░░░░░░  0% PrintMgr
║ ░░░░░░░░░░░░░░░░░░░░░░░░░░░  0% ExtensionMgr
║ █░░░░░░░░░░░░░░░░░░░░░░░░░░  5% AppleTalk
║ █░░░░░░░░░░░░░░░░░░░░░░░░░░  5% FileShare
║ ░░░░░░░░░░░░░░░░░░░░░░░░░░░ 10% ScriptMgr
╚════════════════════════════════════════╝
```

---

## Unblocking Dependencies

### Phase 1 → Enables File Dialogs
```
ListManager (87%)
    ↓ [2-3 hours]
ListManager (100%) ✓
    ↓
FILE DIALOGS FULLY FUNCTIONAL
```

### Phase 2 → Enables Styled UI
```
FontManager (84%)
    ↓ [6-8 hours]
FontManager (100%) ✓
    ↓
ALL TEXT: Bold, Italic, Multiple Sizes
```

### Phase 3 → Enables Print Feature
```
PrintManager (0%)
    ↓ [40-60 hours]
PrintManager (100%) ✓
    ↓ [requires QuickDraw ✓, ResrcMgr ✓]
ALL APPS: Can Print Documents
```

### Phase 4 → Enables Networking
```
AppleTalk Stack (5%)
    ↓ [80-120 hours]
AppleTalk (100%) ✓
    ↓
├─ File Sharing (add 40-60h)
├─ Network Printing (Print + AppleTalk)
└─ Chooser Full Functionality
```

### Phase 5 → Enables System Extension
```
Extension Manager (0%)
    ↓ [50-80 hours]
Extension Manager (100%) ✓
    ↓
├─ Control Panels
├─ INITs at startup
└─ Third-party Extensions
```

---

## Implementation Roadmap

### WEEK 1: UI Polish (12-16 hours)
```
Monday-Tuesday:  ListManager (2-3h)
                 └─ Fix GetTickCount, LSearch
Wednesday-Friday: FontManager (6-8h)
                  └─ Bold, Italic, Sizes
Friday:          ControlManager (4-5h)
                 └─ Font integration, edge cases

OUTCOME: File dialogs 100% functional
         All UI text professionally styled
         Dialog controls fully robust
```

### WEEK 2-3: Print Support (40-60 hours)
```
Full week x2:    Print Manager
                 ├─ Core job management (20-30h)
                 ├─ Print dialog system (10-15h)
                 ├─ Page setup dialog (5-10h)
                 └─ Job spooling (5-10h)

OUTCOME: Printing works from all apps
         Page setup available
         Print preview (optional)
```

### WEEK 4-6: Network Foundation (80-120 hours)
```
Weeks 4-5:       AppleTalk Stack (60-80h)
                 ├─ DDP layer (20h)
                 ├─ NBP discovery (15h)
                 ├─ ATP transactions (15h)
                 └─ ASP sessions (10-20h)

Week 6-7:        File Sharing (40-60h)
                 ├─ AFP protocol (20-30h)
                 ├─ Authentication (10-15h)
                 └─ ACLs/Permissions (10-15h)

OUTCOME: Multi-machine networking
         File sharing between Macs
         Network printing
         Chooser fully functional
```

### WEEK 8-10: System Extensibility (50-80 hours)
```
Weeks 8-9:       Extension Manager (30-50h)
                 ├─ INIT handler (15-20h)
                 ├─ CDEV support (10-15h)
                 └─ Priority loading (5-10h)

Week 9-10:       Control Panels (15-30h)
                 ├─ Control Panel UI (10-15h)
                 └─ Extension management (5-15h)

OUTCOME: System extensible
         Control Panels work
         Third-party INITs supported
         Device drivers installable
```

---

## Risk Assessment

### LOW RISK (Safe to start immediately)
- Phase 1: Core UI completion
  - 80-90% already implemented
  - Clear requirements
  - No architectural changes needed
  - Can parallelize
  - **Risk Level: MINIMAL**

### MEDIUM RISK (Plan carefully, test thoroughly)
- Phase 2: Print Manager
  - Well-defined Mac OS API
  - Some platform abstraction needed
  - Dialog integration required
  - Clear integration points
  - **Risk Level: MODERATE**

### HIGH RISK (Strategic, long-term commitment)
- Phase 3: AppleTalk Stack
  - Complex protocol layers
  - Async I/O architecture
  - Extensive testing needed
  - Potential for subtle bugs
  - **Risk Level: HIGH** (but manageable)

### VERY HIGH RISK (Requires expertise)
- Phase 4: File Sharing
  - Complex protocol semantics
  - Concurrency/locking issues
  - Security implications
  - Hard to test thoroughly
  - **Risk Level: VERY HIGH**

- Phase 5: Extension Manager
  - Deep system knowledge required
  - Trap patching complexity
  - Easy to destabilize system
  - Hard to debug
  - **Risk Level: VERY HIGH**

---

## Estimated Timeline

| Phase | Task | Effort | Risk | Start | Duration |
|-------|------|--------|------|-------|----------|
| 1a | ListManager | 2-3h | Low | Immediate | 1 day |
| 1b | FontManager | 6-8h | Low | Day 2 | 1-2 days |
| 1c | ControlManager | 4-5h | Low | Day 4 | 1 day |
| 1d | TimeManager | 3-4h | Low | Day 5 (opt) | 1 day |
| **TOTAL PHASE 1** | **UI Polish** | **12-16h** | **Low** | **Week 1** | **3-5 days** |
| 2 | Print Manager | 40-60h | Medium | Week 2 | 1-2 weeks |
| 3 | AppleTalk | 80-120h | High | Week 4 | 2-3 weeks |
| 4 | File Sharing | 40-60h | Very High | Week 7 | 1-2 weeks |
| 5 | Extension Manager | 50-80h | Very High | Week 9 | 2-3 weeks |

---

## Success Metrics

### Phase 1 Success
- [ ] All file dialogs fully functional
- [ ] Font rendering includes bold and italic
- [ ] Dialog controls respond smoothly
- [ ] No visual glitches in UI

### Phase 2 Success
- [ ] Print dialog appears
- [ ] Page setup dialog works
- [ ] Can print from SimpleText
- [ ] Can print from MacPaint
- [ ] Print jobs queue properly

### Phase 3 Success
- [ ] Chooser can discover zones
- [ ] Can see other computers
- [ ] Network communication works
- [ ] Connection stability maintained

### Phase 4 Success
- [ ] Can mount remote volumes
- [ ] File access works reliably
- [ ] Permissions enforced
- [ ] Multi-user access safe

### Phase 5 Success
- [ ] INITs load at startup
- [ ] Control Panels available
- [ ] Extensions don't crash system
- [ ] Can enable/disable extensions

---

## Quick Start Checklist

### If Starting Today
```
□ Day 1: ListManager (2-3h) - Quick win
□ Day 2-3: FontManager (6-8h) - Big impact
□ Day 4: ControlManager (4-5h) - Polish
→ Result: 95% complete core UI (16h invested)
```

### If Starting Print Manager
```
□ Hour 1: Read Mac OS Print Manager specs
□ Hour 2-4: Design Print dialog flow
□ Hour 5-30: Implement PrOpen/PrClose/PrJobDialog
□ Hour 31-50: Implement printing backend
□ Hour 51-60: Integration testing
→ Result: Printing fully functional (60h invested)
```

### If Starting AppleTalk (Team Project)
```
Developer A: DDP layer (20h) + ATP (15h)
Developer B: NBP (15h) + ASP (10-20h)
Integration: (10h)
Testing: (10h)
→ Result: Basic networking (80-120h invested)
```

