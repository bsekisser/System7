/*
 * ListSmoke.c - List Manager Smoke Test
 *
 * Tests basic List Manager functionality:
 * - List creation and disposal
 * - Row addition and cell population
 * - Drawing and update
 * - Selection and click handling
 * - Scrolling
 * - Resizing
 *
 * Expected log output:
 *   [LIST] LNew: ...
 *   [LIST] LAddRow: count=60 after=-1 -> rows=60
 *   [LIST] LUpdate: ...
 *   [LIST] LClick: cell(3,0) ...
 *   [LIST] LScroll: dRows=+18 -> topRow=18
 *   [LIST] LSize: new size=(260x220) visRows=13
 *
 * EDGE CASE TESTING GUIDANCE:
 * ---------------------------
 * Beyond this smoke test, verify these edge cases manually or in test harness:
 *
 * 1. Empty list (0 rows):
 *    - LUpdate/LDraw should not crash or draw junk
 *    - LClick should return false (no selection)
 *    - LScroll should clamp to valid range (topRow=0)
 *
 * 2. Single row list:
 *    - LScroll up/down should clamp properly
 *    - Selection should work (click, keyboard navigation)
 *
 * 3. Exactly visible rows (no scroll needed):
 *    - LScroll should be no-op or minimal clamp
 *    - No scrollbar updates if hasVScroll=false
 *
 * 4. Large scroll delta (> row count):
 *    - LScroll(lh, 1000, 0) should clamp to maxScroll
 *    - LScroll(lh, -1000, 0) should clamp to topRow=0
 *
 * 5. Resize to zero or very small:
 *    - LSize(lh, 10, 10) should not crash
 *    - visibleRows should recompute correctly
 *
 * 6. Delete all rows:
 *    - LDelRow to remove all should succeed
 *    - List should behave like empty (case 1)
 *
 * 7. Invalid cell access:
 *    - LSetCell/LGetCell with row >= rowCount should fail gracefully (paramErr)
 *
 * 8. Rapid selection changes:
 *    - Multiple LSetSelect calls should not leak memory or corrupt state
 *
 * 9. BeginUpdate/EndUpdate integration:
 *    - LUpdate called inside BeginUpdate/EndUpdate should clip correctly
 *
 * 10. Window deactivate:
 *     - Selection should render with ltGray when list->active=false
 */

#include "SystemTypes.h"
#include "ListManager/ListManager.h"
#include "WindowManager/WindowManager.h"
#include "ListManager/ListLogging.h"
#include "System71StdLib.h"

/* External functions */
extern WindowPtr NewWindow(void* wStorage, const Rect* boundsRect,
                           ConstStr255Param title, Boolean visible,
                           short procID, WindowPtr behind,
                           Boolean goAwayFlag, long refCon);
extern void ShowWindow(WindowPtr w);
extern void InvalRect(const Rect* r);

/* Smoke test flag - define to enable */
#ifndef LIST_SMOKE_TEST
#define LIST_SMOKE_TEST 0
#endif

#if LIST_SMOKE_TEST

/*
 * RunListSmokeTest - Execute smoke test
 */
void RunListSmokeTest(void)
{
    WindowPtr testWin;
    ListHandle testList;
    ListParams params;
    Rect winRect;
    Rect listRect;
    unsigned char winTitle[32];
    short i;
    Cell c;
    char itemText[32];
    Boolean selChanged;
    short itemHit;
    
    LIST_LOG_INFO("\n[LIST SMOKE] Starting List Manager smoke test\n");
    
    /* Create test window */
    winRect.left = 100;
    winRect.top = 100;
    winRect.right = 420;
    winRect.bottom = 400;
    
    winTitle[0] = 17;  /* Length of "List Manager Test" */
    memcpy(&winTitle[1], "List Manager Test", 17);
    
    testWin = NewWindow(NULL, &winRect, winTitle, true, 0, (WindowPtr)-1, true, 0);
    if (!testWin) {
        LIST_LOG_ERROR("[LIST SMOKE] FAIL: Could not create test window\n");
        return;
    }

    LIST_LOG_INFO("[LIST SMOKE] Created test window\n");
    
    /* Create list */
    listRect.left = 20;
    listRect.top = 40;
    listRect.right = 300;
    listRect.bottom = 300;
    
    params.viewRect = listRect;
    params.cellSizeRect.left = 0;
    params.cellSizeRect.top = 0;
    params.cellSizeRect.right = 200;   /* Cell width */
    params.cellSizeRect.bottom = 16;   /* Cell height */
    params.window = testWin;
    params.hasVScroll = false;
    params.hasHScroll = false;
    params.selMode = lsSingleSel;
    params.refCon = 0x12345678;
    
    testList = LNew(&params);
    if (!testList) {
        LIST_LOG_ERROR("[LIST SMOKE] FAIL: Could not create list\n");
        return;
    }

    LIST_LOG_INFO("[LIST SMOKE] Created list\n");
    
    /* Add 60 rows */
    if (LAddRow(testList, 60, -1) != noErr) {
        LIST_LOG_ERROR("[LIST SMOKE] FAIL: Could not add rows\n");
        LDispose(testList);
        return;
    }
    
    LIST_LOG_INFO("[LIST SMOKE] Added 60 rows\n");
    
    /* Populate cells with "Item N" */
    for (i = 0; i < 60; i++) {
        short len;
        c.v = i;
        c.h = 0;

        /* Create text - simple approach without sprintf */
        itemText[0] = 'I';
        itemText[1] = 't';
        itemText[2] = 'e';
        itemText[3] = 'm';
        itemText[4] = ' ';
        if (i < 10) {
            itemText[5] = '0' + i;
            len = 6;
        } else {
            itemText[5] = '0' + (i / 10);
            itemText[6] = '0' + (i % 10);
            len = 7;
        }

        if (LSetCell(testList, itemText, len, c) != noErr) {
            LIST_LOG_WARN("[LIST SMOKE] WARN: Failed to set cell(%d,0)\n", i);
        }
    }
    
    LIST_LOG_INFO("[LIST SMOKE] Populated all cells\n");
    
    /* Draw list */
    LDraw(testList);
    LIST_LOG_INFO("[LIST SMOKE] Drew list\n");
    
    /* Test click on row 3 */
    {
        Point clickPt;
        clickPt.h = 100;
        clickPt.v = 88;  /* Row 3 at topRow=0, cellHeight=16 -> v=40+(3*16)=88 */
        
        selChanged = LClick(testList, clickPt, 0, &itemHit);
        if (selChanged) {
            LIST_LOG_DEBUG("[LIST SMOKE] Click changed selection: itemHit=%d\n", itemHit);
        } else {
            LIST_LOG_DEBUG("[LIST SMOKE] Click did not change selection\n");
        }
    }
    
    /* Test scroll forward */
    LScroll(testList, 18, 0);
    LIST_LOG_INFO("[LIST SMOKE] Scrolled forward 18 rows\n");
    
    /* Test scroll back */
    LScroll(testList, -10, 0);
    LIST_LOG_INFO("[LIST SMOKE] Scrolled back 10 rows\n");
    
    /* Test resize */
    LSize(testList, 260, 220);
    LIST_LOG_INFO("[LIST SMOKE] Resized list to 260x220\n");
    
    /* Test refCon */
    {
        long refCon = LGetRefCon(testList);
        if (refCon == 0x12345678) {
            LIST_LOG_INFO("[LIST SMOKE] RefCon verified: 0x%08lx\n", refCon);
        } else {
            LIST_LOG_WARN("[LIST SMOKE] WARN: RefCon mismatch: expected 0x12345678, got 0x%08lx\n", refCon);
        }
    }
    
    /* Test selection iteration */
    {
        Cell selCell;
        int selCount = 0;

        /* LGetSelect automatically resets iterator on first call */
        while (LGetSelect(testList, &selCell)) {
            selCount++;
            LIST_LOG_INFO("[LIST SMOKE] Selected cell: row=%d col=%d\n", selCell.v, selCell.h);
            if (selCount > 100) break;  /* Safety */
        }

        LIST_LOG_INFO("[LIST SMOKE] Total selected cells: %d\n", selCount);
    }
    
    /* Test delete rows */
    if (LDelRow(testList, 10, 20) == noErr) {
        LIST_LOG_INFO("[LIST SMOKE] Deleted 10 rows starting at row 20\n");
    }

    /* Final redraw to ensure content is visible */
    LDraw(testList);
    LIST_LOG_INFO("[LIST SMOKE] Final redraw complete\n");

    /* Keep redrawing to ensure content persists */
    /* (Simulates application responding to update events) */
    {
        short refreshCount;
        for (refreshCount = 0; refreshCount < 5; refreshCount++) {
            LUpdate(testList, NULL);
            /* Small delay to let window manager process */
            {
                volatile long delay;
                for (delay = 0; delay < 100000; delay++);
            }
        }
    }

    /* Keep list visible - don't dispose immediately */
    /* Window will stay on screen for manual inspection */
    /* (In production, would dispose on window close event) */

    LIST_LOG_INFO("[LIST SMOKE] Smoke test COMPLETE - List window remains visible\n\n");
    LIST_LOG_INFO("[LIST SMOKE] Window contains list with %d items\n", 50);
    LIST_LOG_INFO("[LIST SMOKE] Close window manually or it will persist in UI\n\n");

    /* Note: testList and testWin NOT disposed - they remain in the UI */
}

#else

void RunListSmokeTest(void)
{
    /* Smoke test disabled */
}

#endif /* LIST_SMOKE_TEST */
