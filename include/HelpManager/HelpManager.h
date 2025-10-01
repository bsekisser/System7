/*
 * HelpManager.h - Mac OS Help Manager API
 *
 * This file provides the complete Mac OS Help Manager API for context-sensitive
 * help balloons and user assistance features. The Help Manager provides balloon
 * help that appears when the user hovers over interface elements.
 *
 * Key features:
 * - Help balloon display and positioning
 * - Context-sensitive help detection
 * - Help resource loading and management
 * - Help navigation and cross-references
 * - Multi-format help content (text, pictures, styled text)
 * - User preference integration
 * - Modern help system abstraction
 */

#ifndef HELPMANAGER_H
#define HELPMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "WindowManager/WindowTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "Menus.h"
#include "ResourceManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Help Manager version */
#define kHMBalloonHelpVersion 0x0002

/* Help Manager error codes (-850 to -874) */

/* Help Manager resource and menu IDs */

/* Dialog item template constants */

/* Options for Help Manager resources */

/* Help content types */

/* Help message types for HMMessageRecord */

/* Resource types for styled TextEdit */
#define kHMTETextResType    'TEXT'   /* Text data without style info */
#define kHMTEStyleResType   'styl'   /* Style information */

/* Item states for extracting help messages */

/* Help resource types */
#define kHMMenuResType      'hmnu'   /* Help resource for menus */
#define kHMDialogResType    'hdlg'   /* Help resource for dialogs */
#define kHMWindListResType  'hwin'   /* Help resource for windows */
#define kHMRectListResType  'hrct'   /* Help resource for rectangles */
#define kHMOverrideResType  'hovr'   /* Help resource for overrides */
#define kHMFinderApplResType 'hfdr'  /* Help resource for Finder */

/* Balloon display methods */

/* Forward declarations */

/* Ptr is defined in MacTypes.h */

/* String resource type for help messages */

/* Help message record - holds various help content types */

} HMMessageRecord;

/* Help Manager function prototypes */

/* Core Help Manager functions */
OSErr HMGetHelpMenuHandle(MenuHandle *mh);
OSErr HMShowBalloon(const HMMessageRecord *aHelpMsg, Point tip,
                   Rect* alternateRect, Ptr tipProc, short theProc,
                   short variant, short method);
OSErr HMRemoveBalloon(void);
Boolean HMGetBalloons(void);
OSErr HMSetBalloons(Boolean flag);
Boolean HMIsBalloon(void);

/* Menu help functions */
OSErr HMShowMenuBalloon(short itemNum, short itemMenuID, long itemFlags,
                       long itemReserved, Point tip, Rect* alternateRect,
                       Ptr tipProc, short theProc, short variant);

/* Help message extraction */
OSErr HMGetIndHelpMsg(ResType whichType, short whichResID, short whichMsg,
                     short whichState, long *options, Point *tip, Rect *altRect,
                     short *theProc, short *variant, HMMessageRecord *aHelpMsg,
                     short *count);
OSErr HMExtractHelpMsg(ResType whichType, short whichResID, short whichMsg,
                      short whichState, HMMessageRecord *aHelpMsg);

/* Font and appearance */
OSErr HMSetFont(short font);
OSErr HMSetFontSize(short fontSize);
OSErr HMGetFont(short *font);
OSErr HMGetFontSize(short *fontSize);

/* Resource management */
OSErr HMSetDialogResID(short resID);
OSErr HMSetMenuResID(short menuID, short resID);
OSErr HMGetDialogResID(short *resID);
OSErr HMGetMenuResID(short menuID, short *resID);

/* Balloon utilities */
OSErr HMBalloonRect(const HMMessageRecord *aHelpMsg, Rect *coolRect);
OSErr HMBalloonPict(const HMMessageRecord *aHelpMsg, PicHandle *coolPict);
OSErr HMGetBalloonWindow(WindowPtr *window);

/* Template scanning */
OSErr HMScanTemplateItems(short whichID, short whichResFile, ResType whichType);

/* Modern help system abstraction */

/* Modern help configuration */

/* Modern help system functions */
OSErr HMConfigureModernHelp(const HMModernHelpConfig *config);
OSErr HMShowModernHelp(const char *helpTopic, const char *anchor);
OSErr HMSearchHelp(const char *searchTerm, Handle *results);
OSErr HMNavigateHelp(const char *linkTarget);

/* Help Manager initialization and shutdown */
OSErr HMInitialize(void);
void HMShutdown(void);

/* Private Help Manager functions (for internal use) */
#ifdef HELP_MANAGER_PRIVATE
OSErr HMCountDITLHelpItems(short ditlID, short *count);
OSErr HMModalDialogMenuSetup(short menuID);
OSErr HMInvalidateSavedBits(WindowPtr theWindow, RgnHandle invalRgn);
OSErr HMTrackModalHelpItems(void);
OSErr HMBalloonBulk(void);
OSErr HMInitHelpMenu(void);
OSErr HMDrawBalloonFrame(const Rect *balloonRect, short variant);
OSErr HMSetupBalloonRgns(const Rect *balloonRect, Point tip,
                        RgnHandle balloonRgn, RgnHandle structRgn,
                        short variant);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HELPMANAGER_H */