/*
 * HelpResources.h - Help Resource Loading and Management
 *
 * This file defines structures and functions for loading, managing, and
 * caching help resources including hmnu, hdlg, hrct, hovr, and hfdr resources.
 */

#ifndef HELPRESOURCES_H
#define HELPRESOURCES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "ResourceManager.h"
#include "HelpManager.h"
#include "HelpContent.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Help resource types */
#define kHMMenuResource     'hmnu'   /* Menu help resources */
#define kHMDialogResource   'hdlg'   /* Dialog help resources */
#define kHMWindowResource   'hwin'   /* Window help resources */
#define kHMRectResource     'hrct'   /* Rectangle help resources */
#define kHMOverrideResource 'hovr'   /* Override help resources */
#define kHMFinderResource   'hfdr'   /* Finder help resources */

/* Resource loading flags */

/* Resource cache entry */

/* Resource loader state */

/* Menu help resource structure */

/* Dialog help resource structure */
                   /* Resource version */
    short offset;                    /* Item offset */
    long options;                    /* Resource options */
    short procID;                    /* Balloon procedure ID */
    short variant;                   /* Balloon variant */
    short itemCount;                 /* Number of dialog items */
    HMMessageRecord missingMessage;  /* Message for missing items */
    Point missingTip;                /* Tip point for missing items */
    Rect missingHotRect;             /* Hot rect for missing items */
    struct {
        HMMessageRecord message;     /* Item help message */
        Point tip;                   /* Item tip point */
        Rect hotRect;                /* Item hot rectangle */
    } *itemHelp;                     /* Array of item help */
} HMDialogResource;

/* Window help resource structure */
                   /* Resource version */
    long options;                    /* Resource options */
    short templateCount;             /* Number of templates */
    struct {
        short resourceID;            /* Template resource ID */
        ResType resourceType;        /* Template resource type */
        short titleLength;           /* Title string length */
        char *titleString;           /* Window title string */
    } *templates;                    /* Array of templates */
} HMWindowResource;

/* Rectangle help resource structure */
                   /* Resource version */
    long options;                    /* Resource options */
    short procID;                    /* Balloon procedure ID */
    short variant;                   /* Balloon variant */
    short rectCount;                 /* Number of rectangles */
    struct {
        HMMessageRecord message;     /* Rectangle help message */
        Point tip;                   /* Rectangle tip point */
        Rect hotRect;                /* Hot rectangle */
    } *rectHelp;                     /* Array of rectangle help */
} HMRectResource;

/* Override help resource structure */

/* Finder help resource structure */

/* Resource loading functions */
OSErr HMResourceInit(short maxCacheEntries, long memoryLimit);
void HMResourceShutdown(void);

OSErr HMResourceLoad(ResType resourceType, short resourceID, short resourceFile,
                    long flags, Handle *resourceHandle);

OSErr HMResourceLoadMenu(short menuID, HMMenuResource **menuResource);
OSErr HMResourceLoadDialog(short dialogID, HMDialogResource **dialogResource);
OSErr HMResourceLoadWindow(short windowID, HMWindowResource **windowResource);
OSErr HMResourceLoadRect(short rectID, HMRectResource **rectResource);
OSErr HMResourceLoadOverride(short overrideID, HMOverrideResource **overrideResource);
OSErr HMResourceLoadFinder(short finderID, HMFinderResource **finderResource);

/* Resource validation functions */
Boolean HMResourceValidate(ResType resourceType, Handle resourceHandle);
OSErr HMResourceCheckVersion(Handle resourceHandle, short *version);
OSErr HMResourceCheckFormat(ResType resourceType, Handle resourceHandle);

/* Resource parsing functions */
OSErr HMResourceParseMenu(Handle resourceHandle, HMMenuResource **menuResource);
OSErr HMResourceParseDialog(Handle resourceHandle, HMDialogResource **dialogResource);
OSErr HMResourceParseWindow(Handle resourceHandle, HMWindowResource **windowResource);
OSErr HMResourceParseRect(Handle resourceHandle, HMRectResource **rectResource);
OSErr HMResourceParseOverride(Handle resourceHandle, HMOverrideResource **overrideResource);
OSErr HMResourceParseFinder(Handle resourceHandle, HMFinderResource **finderResource);

/* Message extraction functions */
OSErr HMResourceExtractMessage(ResType resourceType, short resourceID,
                             short messageIndex, short messageState,
                             HMMessageRecord *message, Point *tip,
                             Rect *hotRect, long *options);

OSErr HMResourceExtractMenuMessage(short menuID, short itemNumber, short itemState,
                                 HMMessageRecord *message);

OSErr HMResourceExtractDialogMessage(short dialogID, short itemNumber, short itemState,
                                   HMMessageRecord *message, Point *tip, Rect *hotRect);

OSErr HMResourceExtractRectMessage(short rectID, short rectIndex,
                                 HMMessageRecord *message, Point *tip, Rect *hotRect);

/* Resource caching functions */
OSErr HMResourceCacheStore(ResType resourceType, short resourceID,
                         Handle resourceHandle);
OSErr HMResourceCacheRetrieve(ResType resourceType, short resourceID,
                            Handle *resourceHandle);
OSErr HMResourceCacheRemove(ResType resourceType, short resourceID);
void HMResourceCacheClear(void);

OSErr HMResourceCacheGetStatistics(short *entryCount, long *memoryUsed,
                                 long *hitCount, long *missCount);
OSErr HMResourceCacheSetLimits(short maxEntries, long maxMemory);

/* Resource preloading functions */
OSErr HMResourcePreloadMenu(short menuID);
OSErr HMResourcePreloadDialog(short dialogID);
OSErr HMResourcePreloadWindow(short windowID);
OSErr HMResourcePreloadApplication(short applicationID);

OSErr HMResourcePreloadFromList(ResType resourceType, short *resourceIDs,
                              short resourceCount);

/* Resource file management */
OSErr HMResourceAddFile(short fileRefNum, Boolean makeDefault);
OSErr HMResourceRemoveFile(short fileRefNum);
OSErr HMResourceSetDefaultFile(short fileRefNum);
short HMResourceGetDefaultFile(void);

OSErr HMResourceSearchFiles(ResType resourceType, short resourceID,
                          short *fileRefNum, Boolean *found);

/* Language support functions */
OSErr HMResourceSetLanguage(const char *languageCode, const char *fallbackCode);
OSErr HMResourceGetLanguage(char *languageCode, char *fallbackCode);

OSErr HMResourceLoadLocalized(ResType resourceType, short resourceID,
                            const char *languageCode, Handle *resourceHandle);

Boolean HMResourceHasLocalization(ResType resourceType, short resourceID,
                                 const char *languageCode);

/* Template scanning functions */
OSErr HMResourceScanTemplates(short templateID, short resourceFile,
                            ResType templateType);

OSErr HMResourceScanDialog(short dialogID, short resourceFile);
OSErr HMResourceScanMenu(short menuID, short resourceFile);
OSErr HMResourceScanWindow(short windowID, short resourceFile);

/* Resource building functions */
OSErr HMResourceBuildMenu(const HMMenuResource *menuResource, Handle *resourceHandle);
OSErr HMResourceBuildDialog(const HMDialogResource *dialogResource, Handle *resourceHandle);
OSErr HMResourceBuildWindow(const HMWindowResource *windowResource, Handle *resourceHandle);
OSErr HMResourceBuildRect(const HMRectResource *rectResource, Handle *resourceHandle);

/* Resource disposal functions */
void HMResourceDisposeMenu(HMMenuResource *menuResource);
void HMResourceDisposeDialog(HMDialogResource *dialogResource);
void HMResourceDisposeWindow(HMWindowResource *windowResource);
void HMResourceDisposeRect(HMRectResource *rectResource);
void HMResourceDisposeOverride(HMOverrideResource *overrideResource);
void HMResourceDisposeFinder(HMFinderResource *finderResource);

/* Resource utilities */
OSErr HMResourceGetInfo(ResType resourceType, short resourceID,
                      ResType *actualType, short *actualID,
                      Str255 resourceName, short *resourceFile);

OSErr HMResourceSetInfo(ResType resourceType, short resourceID,
                      const Str255 resourceName);

Boolean HMResourceExists(ResType resourceType, short resourceID, short resourceFile);

short HMResourceCount(ResType resourceType, short resourceFile);

OSErr HMResourceGetIDList(ResType resourceType, short resourceFile,
                        short **resourceIDs, short *count);

/* Resource error handling */
OSErr HMResourceGetLastError(void);
void HMResourceClearError(void);
const char* HMResourceGetErrorString(OSErr error);

/* Modern resource support */
OSErr HMResourceLoadFromBundle(const char *bundlePath, ResType resourceType,
                             short resourceID, Handle *resourceHandle);

OSErr HMResourceLoadFromURL(const char *url, ResType resourceType,
                          short resourceID, Handle *resourceHandle);

OSErr HMResourceRegisterLoader(ResType resourceType,
                             OSErr (*loader)(short resourceID, Handle *resourceHandle));

/* Resource debugging */
OSErr HMResourceDumpCache(void);
OSErr HMResourceValidateCache(void);
OSErr HMResourceGetDebugInfo(char *debugInfo, long maxLength);

#ifdef __cplusplus
}
#endif

#endif /* HELPRESOURCES_H */