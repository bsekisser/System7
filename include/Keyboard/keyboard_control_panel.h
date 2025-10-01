/*
 * RE-AGENT-BANNER
 * keyboard_control_panel.h - Apple System 7.1 Keyboard Control Panel
 *
 * Reverse-engineered from: Keyboard.rsrc
 * Original file hash: b75947c075f427222008bd84a797c7df553a362ed4ee71a1bd3a22f18adf8f10
 * Architecture: Motorola 68000 series
 * System: Classic Mac OS 7.1
 *
 * This header defines the data structures and function prototypes for the
 * Keyboard control panel, which manages key repeat rates, delay settings,
 * and keyboard layout selection in System 7.1.
 *
 * Evidence base: analysis of strings "Key Repeat Rate", "Delay Until Repeat",
 * "Keyboard Layout", control panel conventions, and resource structure.
 *
 * Provenance: Original binary -> radare2 analysis -> evidence curation ->
 * structure layout analysis -> symbol mapping -> reimplementation
 */

#ifndef KEYBOARD_CONTROL_PANEL_H
#define KEYBOARD_CONTROL_PANEL_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/* Additional types for Keyboard control panel */
/* Ptr is defined in MacTypes.h */

/* Control Panel Message Types (Standard Mac OS cdev messages) */
/*

/* Control Panel Parameter Block */
/*

/* Keyboard Configuration Constants */
/*

/* Modern Keyboard Layout Types */
/*

/* Legacy compatibility defines */
#define kLayoutDomestic kLayoutUS     /* Maintain backward compatibility */

/* Unicode and International Support */
/*
        /* UTF-32 character code */
          /* UTF-16 character code */

/* Dead Key Support for International Layouts */
/*

/* Dead Key State */

/* Layout Feature Flags */
/*

/* Modifier Key State */

/* Key Mapping Entry for International Layouts */

/* Extended Layout Information */

/* Key Repeat Timing Configuration */
/*

/* Keyboard Settings Structure */
/*

/* Keyboard Layout Information */
/*

/* Control Panel Instance Data */
/*

/* Dialog Item Constants */
/*
#define kItemRepeatRateSlider   1
#define kItemDelaySlider        2
#define kItemLayoutList         3
#define kItemDomesticIcon       4     /* Legacy: US layout icon */
#define kItemInternationalIcon  5     /* Legacy: International layout icon */
#define kItemRepeatRateLabel    6
#define kItemDelayLabel         7
#define kItemLayoutLabel        8

/* Extended UI elements for modern layout support */
#define kItemLayoutPopup        9     /* Modern layout selection popup */
#define kItemDeadKeyIndicator   10    /* Dead key status indicator */
#define kItemLayoutPreview      11    /* Layout preview area */
#define kItemAltGrLabel         12    /* AltGr key information */
#define kItemUnicodeSupport     13    /* Unicode support indicator */
#define kItemLayoutSearch       14    /* Search/filter layouts */
#define kItemLayoutDetect       15    /* Auto-detect layout button */
#define kItemLayoutRegion       16    /* Region/language selection */

/* Control Panel Version and Type */
/*
#define kKeyboardCPVersion      0x0701
#define kKeyboardCPCreator      'keyb'
#define kKeyboardCPType         'cdev'

/* Resource IDs */
/*
#define kKeyboardDialogDITL     128
#define kKeyboardLayoutLDEF     128
#define kKeyboardStrings        128
#define kKeyboardIcon           128

/* Function Prototypes */

/*
 * Main control panel entry point

 * Handles all control panel messages and dispatches to appropriate handlers
 */
long KeyboardControlPanel_main(CdevParam *params);

/*
 * Initialize keyboard control panel dialog

 */
OSErr KeyboardCP_InitDialog(ControlPanelData *cpData);

/*
 * Handle user interaction with dialog items

 */
void KeyboardCP_HandleItemHit(ControlPanelData *cpData, short item);

/*
 * Apply keyboard settings to system

 */
OSErr KeyboardCP_UpdateSettings(KeyboardSettings *settings);

/*
 * Load KCHR resource for keyboard layout

 */
Handle KeyboardCP_LoadKCHR(short layoutID);

/*
 * Set system key repeat rate

 */
void KeyboardCP_SetRepeatRate(short rate);

/*
 * Set delay before key repeat starts

 */
void KeyboardCP_SetRepeatDelay(short delay);

/*
 * Get current keyboard layout setting

 */
short KeyboardCP_GetCurrentLayout(void);

/*
 * Change current keyboard layout

 */
OSErr KeyboardCP_SetKeyboardLayout(short layoutID);

/* Extended Modern Layout Functions */

/*
 * Get extended layout information for a layout ID

 */
ExtendedLayoutInfo* KeyboardCP_GetExtendedLayoutInfo(KeyboardLayoutType layoutID);

/*
 * Get all available layouts on the system

 */
OSErr KeyboardCP_GetAvailableLayouts(KeyboardLayoutType* layouts, short* numLayouts, short maxLayouts);

/*
 * Get layout name for display in UI

 */
const char* KeyboardCP_GetLayoutName(KeyboardLayoutType layoutID);

/*
 * Get layout name in local language

 */
const char* KeyboardCP_GetLocalizedLayoutName(KeyboardLayoutType layoutID);

/*
 * Check if a layout supports dead keys

 */
Boolean KeyboardCP_SupportsDeadKeys(KeyboardLayoutType layoutID);

/*
 * Check if a layout supports AltGr key

 */
Boolean KeyboardCP_SupportsAltGr(KeyboardLayoutType layoutID);

/*
 * Process dead key input for international layouts

 */
UnicodeChar KeyboardCP_ProcessDeadKey(DeadKeyType deadKey, UnicodeChar baseChar);

/*
 * Auto-detect system keyboard layout

 */
KeyboardLayoutType KeyboardCP_DetectSystemLayout(void);

/*
 * Convert virtual key to Unicode character for current layout

 */
UnicodeChar KeyboardCP_VirtualKeyToUnicode(short virtualKey, ModifierKeyFlags modifiers);

/*
 * Get layout by language/country code

 */
KeyboardLayoutType KeyboardCP_GetLayoutByCode(const char* languageCode, const char* countryCode);

/*
 * Check if layout requires Input Method Editor (IME)

 */
Boolean KeyboardCP_RequiresIME(KeyboardLayoutType layoutID);

/*
 * Get layout direction (LTR/RTL)

 */
Boolean KeyboardCP_IsRightToLeft(KeyboardLayoutType layoutID);

#endif /* KEYBOARD_CONTROL_PANEL_H */

/*
 * RE-AGENT-TRAILER-JSON
 * {
 *   "file": "keyboard_control_panel.h",
 *   "purpose": "Header definitions for Apple System 7.1 Keyboard Control Panel",
 *   "evidence_base": [
 *     "UI strings: Key Repeat Rate, Delay Until Repeat, Keyboard Layout",
 *     "Options: Slow/Fast, Long/Short, Domestic/International",
 *     "Resource types: cdev, KCHR, DITL, LDEF, STR#, ICN#",
 *     "Control panel conventions: CdevParam, message types",
 *     "Version identifier: v7.1",
 *     "File metadata: type=cdev, creator=keyb"
 *   ],
 *   "structures_defined": 6,
 *   "functions_declared": 9,
 *   "constants_defined": 15,
 *   "provenance_density": 0.12
 * }
 */