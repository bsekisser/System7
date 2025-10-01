/* #include "SystemTypes.h" */
#include "QuickDrawConstants.h"
// #include "CompatibilityFix.h" // Removed
/*
 * RE-AGENT-BANNER
 * QuickDraw Core Implementation
 * Clean-room reimplementation from ROM analysis
 *
 * SOURCE: Quadra 800 ROM (1MB, 1993 release)
 * Architecture: 68k Mac OS System 7
 * Evidence sources: trap analysis, Mac OS Toolbox reference, binary analysis
 *
 * This file implements core QuickDraw port management functions.
 * Functions are implemented to match the original 68k Mac OS behavior.
 */

#include "SystemTypes.h"
#include "QuickDrawConstants.h"

#include "QuickDraw/quickdraw.h"
#include "QuickDraw/quickdraw_types.h"
/* #include "mac_memory.h"
 */

/* Global current graphics port */
GrafPtr thePort = NULL;

/* Standard patterns - Evidence: These are standard Mac OS patterns */
static const Pattern kBlackPattern = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const Pattern kWhitePattern = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const Pattern kGrayPattern = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
static const Pattern kLtGrayPattern = {0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22};
static const Pattern kDkGrayPattern = {0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD};

/*
 * InitGraf - Initialize QuickDraw graphics system
 * Trap: 0xA86E

 */
void InitGraf(GrafPtr port)
{
    if (port == NULL) return;

    /* Initialize the graphics port */
    port->device = 0;

    /* Initialize port bitmap */
    port->portBits.baseAddr = NULL;
    port->portBits.rowBytes = 0;
    SetRect(&port->portBits.bounds, 0, 0, 0, 0);

    /* Set port rectangle to empty */
    SetRect(&port->portRect, 0, 0, 0, 0);

    /* Initialize regions to NULL - will be allocated later */
    port->visRgn = NULL;
    port->clipRgn = NULL;

    /* Set default patterns */
    port->bkPat = kWhitePattern;
    port->fillPat = kBlackPattern;

    /* Initialize pen state */
    SetPt(&port->pnLoc, 0, 0);
    SetPt(&port->pnSize, 1, 1);
    port->pnMode = srcCopy;
    port->pnPat = kBlackPattern;
    port->pnVis = 0;

    /* Initialize text state */
    port->txFont = 0;         /* System font */
    port->txFace = normal;
    port->txMode = srcCopy;
    port->txSize = 12;        /* Default 12 point */
    port->spExtra = 0;        /* No extra space */

    /* Initialize color state */
    port->fgColor = 0x00000000;  /* Black */
    port->bkColor = 0xFFFFFFFF;  /* White */
    port->colrBit = 0;
    port->patStretch = 0;

    /* Initialize save handles */
    port->picSave = NULL;
    port->rgnSave = NULL;
    port->polySave = NULL;
    port->grafProcs = NULL;

    /* Set as current port */
    thePort = port;
}

/*
 * OpenPort - Initialize and open a graphics port
 * Trap: 0xA86F

 */
void OpenPort(GrafPtr port)
{
    if (port == NULL) return;

    /* Initialize the port with default values */
    InitGraf(port);

    /* Allocate default regions */
    port->visRgn = NewRgn();
    port->clipRgn = NewRgn();

    if (port->visRgn) {
        SetEmptyRgn(port->visRgn);
    }
    if (port->clipRgn) {
        SetEmptyRgn(port->clipRgn);
    }
}

/*
 * ClosePort - Close a graphics port
 * Trap: 0xA87D

 */
void ClosePort(GrafPtr port)
{
    if (port == NULL) return;

    /* Dispose of allocated regions */
    if (port->visRgn) {
        DisposeRgn(port->visRgn);
        port->visRgn = NULL;
    }
    if (port->clipRgn) {
        DisposeRgn(port->clipRgn);
        port->clipRgn = NULL;
    }

    /* Clear current port if this was it */
    if (thePort == port) {
        thePort = NULL;
    }
}

/*
 * SetPort - Set the current graphics port
 * Trap: 0xA873

 */
void SetPort(GrafPtr port)
{
    thePort = port;
}

/*
 * GetPort - Get the current graphics port
 * Trap: 0xA874

 */
void GetPort(GrafPtr* port)
{
    if (port != NULL) {
        *port = thePort;
    }
}

/*
 * GetPen - Get current pen position

 */
void GetPen(Point* pt)
{
    if (pt != NULL && thePort != NULL) {
        *pt = thePort->pnLoc;
    }
}

/*
 * PenNormal - Set pen to normal state

 */
void PenNormal(void)
{
    if (thePort == NULL) return;

    SetPt(&thePort->pnSize, 1, 1);
    thePort->pnMode = srcCopy;
    thePort->pnPat = kBlackPattern;
}

/*
 * PenSize - Set pen size

 */
void PenSize(short width, short height)
{
    if (thePort == NULL) return;

    SetPt(&thePort->pnSize, width, height);
}

/*
 * PenMode - Set pen transfer mode

 */
void PenMode(short mode)
{
    if (thePort == NULL) return;

    thePort->pnMode = mode;
}

/*
 * PenPat - Set pen pattern

 */
void PenPat(const Pattern* pat)
{
    if (thePort == NULL || pat == NULL) return;

    thePort->pnPat = *pat;
}

/*
 * BackPat - Set background pattern

 */
void BackPat(const Pattern* pat)
{
    if (thePort == NULL || pat == NULL) return;

    thePort->bkPat = *pat;
}

/*
 * ForeColor - Set foreground color

 */
void ForeColor(long color)
{
    if (thePort == NULL) return;

    thePort->fgColor = color;
}

/*
 * BackColor - Set background color

 */
void BackColor(long color)
{
    if (thePort == NULL) return;

    thePort->bkColor = color;
}

/*
 * ColorBit - Set color bit

 */
void ColorBit(short whichBit)
{
    if (thePort == NULL) return;

    thePort->colrBit = whichBit;
}

/*
