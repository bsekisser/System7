/* RE-AGENT-BANNER
 * TextEdit Header - Apple System 7.1 TextEdit Manager
 *
 * implemented based on: System.rsrc
 * ROM disassembly
 * Architecture: m68k (68000)
 * ABI: classic_macos
 *

 * Mappings: mappings.textedit.json
 * Layouts: layouts.curated.textedit.json
 *
 * TextEdit is the Mac OS Toolbox component responsible for text editing
 * functionality. It provides text input, editing, display, and clipboard
 * operations for applications. This implementation is based on System 7.1.
 */

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/* Forward declarations */

/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Basic Mac OS types */

/* Ptr is defined in MacTypes.h */
/* Style is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* TextEdit constants - Evidence: evidence.curated.textedit.json */
#define teJustLeft   0   /* Left justification */
#define teJustCenter 1   /* Center justification */
#define teJustRight  (-1) /* Right justification */
#define teFAutoScroll 0  /* Auto-scroll feature flag */

/* TextEdit Record Structure - Layout: layouts.curated.textedit.json:TERec */

/* TextEdit Style Record - Layout: layouts.curated.textedit.json:TEStyleRec */

/* Handle is defined in MacTypes.h */

/* TextEdit Function Prototypes - Evidence: evidence.curated.textedit.json */

/* Initialization and lifecycle - Trap: 0xA9CC, Evidence: offset 0x00003780 */
void TEInit(void);

/* Create new TextEdit record - Trap: 0xA9D2, Evidence: offset 0x00003f00 */
TEHandle TENew(Rect *destRect, Rect *viewRect);

/* Dispose TextEdit record - Trap: 0xA9CD, Evidence: offset 0x00004180 */
void TEDispose(TEHandle hTE);

/* Text content management - Trap: 0xA9CF, Evidence: offset 0x00004280 */
void TESetText(void *text, SInt32 length, TEHandle hTE);

/* Get text handle - Trap: 0xA9CB, Evidence: offset 0x00004350 */
CharsHandle TEGetText(TEHandle hTE);

/* User interaction - Trap: 0xA9D4, Evidence: offset 0x00004450 */
void TEClick(Point pt, SInt16 extend, TEHandle hTE);

/* Keyboard input - Trap: 0xA9DC, Evidence: offset 0x00004650 */
void TEKey(SInt16 key, TEHandle hTE);

/* Clipboard operations - Traps: 0xA9D6/0xA9D5/0xA9D7, Evidence: offsets 0x00004800-0x00004920 */
void TECut(TEHandle hTE);
void TECopy(TEHandle hTE);
void TEPaste(TEHandle hTE);

/* Text editing - Trap: 0xA9D3, Evidence: offset 0x00004a00 */
void TEDelete(TEHandle hTE);

/* Text insertion - Trap: 0xA9DE, Evidence: offset 0x00004b00 */
void TEInsert(void *text, SInt32 length, TEHandle hTE);

/* Selection management - Trap: 0xA9D1, Evidence: offset 0x00004c50 */
void TESetSelect(SInt32 selStart, SInt32 selEnd, TEHandle hTE);

/* Activation state - Traps: 0xA9D8/0xA9D9, Evidence: offsets 0x00004d20-0x00004d80 */
void TEActivate(TEHandle hTE);
void TEDeactivate(TEHandle hTE);

/* Idle processing for caret - Trap: 0xA9DA, Evidence: offset 0x00004e00 */
void TEIdle(TEHandle hTE);

/* Display update - Trap: 0xA9D3, Evidence: offset 0x00004f00 */
void TEUpdate(void *updateRgn, TEHandle hTE);

/* Scrolling - Trap: 0xA9DD, Evidence: offset 0x00005100 */
void TEScroll(SInt16 dh, SInt16 dv, TEHandle hTE);

/* Simple text display - Trap: 0xA9CE, Evidence: offset 0x00005200 */
void TETextBox(void *text, SInt32 length, Rect *box, SInt16 just);

#endif /* TEXTEDIT_H */

/* RE-AGENT-TRAILER-JSON
{
  "component": "TextEdit",
  "file": "include/textedit.h",
  "functions": 19,
  "structures": 2,
  "constants": 4,
  "evidence_refs": 19,
  "provenance_density": 0.087,
  "generated": "2024-09-18"
}
*/