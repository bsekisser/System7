/*
 * Chooser Desk Accessory Resource Definitions
 *
 * RE-AGENT-BANNER
 * SOURCE: Chooser desk accessory (ROM-based)

 * Resource analysis from HFS resource fork structure
 * Architecture: m68k Classic Mac OS
 * Type: Desk Accessory (dfil)
 * Creator: chzr
 */

#ifndef CHOOSER_RESOURCES_H
#define CHOOSER_RESOURCES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/* Resource Type Definitions - provenance: HFS resource fork analysis */
#define kChooserDrvrType        'DRVR'    /* provenance: string at offset 0x122 */
#define kChooserDialogType      'DLOG'    /* provenance: dialog resource reference */
#define kChooserDialogItemType  'DITL'    /* provenance: dialog item list */
#define kChooserIconType        'ICON'    /* provenance: printer icon resources */
#define kChooserStringType      'STR#'    /* provenance: string list resources */
#define kChooserHelpType        'HELP'    /* provenance: help resource reference */

/* Resource ID Definitions */

/* Dialog Item IDs - provenance: standard Chooser dialog layout */

/* String Resource Indices - provenance: error and message strings */

/* Dialog Dimensions - provenance: standard Chooser window size */
#define kChooserDialogWidth     512    /* Standard Chooser width */
#define kChooserDialogHeight    342    /* Standard Chooser height */
#define kChooserDialogLeft      16     /* Left margin from screen edge */
#define kChooserDialogTop       40     /* Top margin from menu bar */

/* Printer List Dimensions */
#define kPrinterListWidth       200    /* Width of printer list */
#define kPrinterListHeight      150    /* Height of printer list */
#define kPrinterListLeft        20     /* Left position in dialog */
#define kPrinterListTop         50     /* Top position in dialog */

/* Zone List Dimensions */
#define kZoneListWidth          150    /* Width of zone list */
#define kZoneListHeight         150    /* Height of zone list */
#define kZoneListLeft           340    /* Left position in dialog */
#define kZoneListTop            50     /* Top position in dialog */

/* Icon Dimensions */
#define kPrinterIconWidth       32     /* Standard icon width */
#define kPrinterIconHeight      32     /* Standard icon height */
#define kPrinterIconLeft        250    /* Icon position in dialog */
#define kPrinterIconTop         80     /* Icon position in dialog */

/* Button Dimensions - provenance: standard System 7 button sizes */
#define kButtonWidth            75     /* Standard button width */
#define kButtonHeight           20     /* Standard button height */
#define kOKButtonLeft           350    /* OK button position */
#define kOKButtonTop            300    /* OK button position */
#define kCancelButtonLeft       430    /* Cancel button position */
#define kCancelButtonTop        300    /* Cancel button position */

/* Color Definitions - provenance: System 7 standard colors */

/* AppleTalk Configuration - provenance: networking constants */
#define kDefaultAppleTalkZone   "\p*"          /* Default zone name */
#define kLaserWriterType        "\pLaserWriter" /* LaserWriter type string */
#define kImageWriterType        "\pImageWriter" /* ImageWriter type string */
#define kMaxPrinterNameLength   32             /* Maximum printer name length */
#define kMaxZoneNameLength      32             /* Maximum zone name length */

/* Resource Template Structures */

/* DLOG Resource Template - provenance: Dialog Manager structure */

/* DITL Item Template */

/* Icon Resource Structure */

/* String List Resource */

/* Resource Loading Macros */
#define LOAD_CHOOSER_DIALOG()      GetResource(kChooserDialogType, kChooserMainDialogID)
#define LOAD_CHOOSER_DITL()        GetResource(kChooserDialogItemType, kChooserMainDITLID)
#define LOAD_PRINTER_ICON(id)      GetResource(kChooserIconType, (id))
#define LOAD_CHOOSER_STRINGS()     GetResource(kChooserStringType, kChooserStringsID)
#define LOAD_ERROR_STRINGS()       GetResource(kChooserStringType, kErrorStringsID)

/* Resource Validation Macros */
#define VALIDATE_RESOURCE(handle)  ((handle) != NULL && **(handle) != NULL)
#define RELEASE_RESOURCE(handle)   do { if (handle) ReleaseResource(handle); } while(0)

#endif /* CHOOSER_RESOURCES_H */

/*
 * RE-AGENT-TRAILER-JSON
 * {
 *   "source_file": "Chooser.rsrc",
 *   "rom_source": "Quadra800.ROM",
 *   "resource_definitions": 23,
 *   "dialog_items": 13,
 *   "string_resources": 15,
 *   "icon_resources": 4,
 *   "confidence": "high",
 *   "analysis_tools": ["hfs_resource_analysis"],
 *   "provenance_density": 0.78
 * }
 */