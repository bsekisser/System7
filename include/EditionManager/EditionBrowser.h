/*
 * EditionBrowser.h
 *
 * Edition Browser API for Edition Manager
 * Provides UI for finding, selecting, and managing edition containers
 */

#ifndef __EDITION_BROWSER_H__
#define __EDITION_BROWSER_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "EditionManager/EditionManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Edition Preview Information
 */

/*
 * Edition Browser Filter
 */

/*
 * Edition Browser Callback
 */

/*
 * Edition Selection Result
 */

/*
 * Core Browser Functions
 */

/* Browse for edition containers */
OSErr BrowseForEdition(const char* startPath, EditionContainerSpec* selectedContainer);

/* Browse with filter criteria */
OSErr BrowseForEditionWithFilter(const char* startPath,
                                const EditionBrowserFilter* filter,
                                EditionSelectionResult* result);

/* Get list of available editions in directory */
OSErr GetAvailableEditions(const char* directoryPath,
                          EditionContainerSpec** editions,
                          SInt32* editionCount);

/* Filter editions by format support */
OSErr FilterEditionsByFormat(const EditionContainerSpec* editions,
                            SInt32 editionCount,
                            FormatType format,
                            EditionContainerSpec** filteredEditions,
                            SInt32* filteredCount);

/* Get preview information for edition */
OSErr GetEditionPreviewInfo(const EditionContainerSpec* container,
                           EditionPreviewInfo* previewInfo);

/*
 * Dialog Functions
 */

/* Show standard subscriber dialog */
OSErr ShowSubscriberBrowserDialog(EditionSelectionResult* result);

/* Show standard publisher dialog */
OSErr ShowPublisherBrowserDialog(NewPublisherReply* reply);

/* Show custom edition browser dialog */
OSErr ShowCustomEditionBrowser(const char* title,
                              const EditionBrowserFilter* filter,
                              EditionSelectionResult* result);

/*
 * Browser Configuration
 */

/* Set browser filter criteria */
OSErr SetEditionBrowserFilter(FormatType format, Boolean showPreview);

/* Set browser callback */
OSErr SetEditionBrowserCallback(EditionBrowserCallback callback, void* userData);

/* Set browser window properties */

OSErr SetBrowserWindowProperties(const BrowserWindowProps* props);

/* Get browser window properties */
OSErr GetBrowserWindowProperties(BrowserWindowProps* props);

/*
 * Recent Editions Management
 */

/* Add edition to recent list */
OSErr AddToRecentEditions(const EditionContainerSpec* container);

/* Get recent editions list */
OSErr GetRecentEditions(EditionContainerSpec** recentEditions,
                       SInt32* editionCount);

/* Clear recent editions list */
OSErr ClearRecentEditions(void);

/* Set maximum recent editions count */
OSErr SetMaxRecentEditions(SInt32 maxCount);

/*
 * Edition Search and Discovery
 */

/* Search for editions by name pattern */
OSErr SearchEditionsByName(const char* searchPath,
                          const char* namePattern,
                          EditionContainerSpec** foundEditions,
                          SInt32* editionCount);

/* Search for editions by content */
OSErr SearchEditionsByContent(const char* searchPath,
                             FormatType format,
                             const void* searchData,
                             Size dataSize,
                             EditionContainerSpec** foundEditions,
                             SInt32* editionCount);

/* Search for editions by metadata */
OSErr SearchEditionsByMetadata(const char* searchPath,
                              OSType creator,
                              OSType type,
                              TimeStamp newerThan,
                              EditionContainerSpec** foundEditions,
                              SInt32* editionCount);

/*
 * Edition Organization
 */

/* Get edition categories */

OSErr GetEditionCategories(const char* searchPath,
                          EditionCategory** categories,
                          SInt32* categoryCount);

/* Create edition alias/shortcut */
OSErr CreateEditionAlias(const EditionContainerSpec* container,
                        const char* aliasPath);

/* Resolve edition alias */
OSErr ResolveEditionAlias(const char* aliasPath,
                         EditionContainerSpec* container);

/*
 * Edition Validation and Repair
 */

/* Validate edition accessibility */
OSErr ValidateEditionAccess(const EditionContainerSpec* container,
                           Boolean* isAccessible,
                           char* errorMessage,
                           Size messageSize);

/* Check for broken edition links */
OSErr CheckEditionLinks(const EditionContainerSpec* editions,
                       SInt32 editionCount,
                       Boolean** brokenLinks);

/* Repair broken edition links */
OSErr RepairEditionLinks(EditionContainerSpec* editions,
                        SInt32 editionCount,
                        SInt32* repairedCount);

/*
 * Preview Generation
 */

/* Generate preview for edition */
OSErr GenerateEditionPreview(const EditionContainerSpec* container,
                            FormatType previewFormat,
                            Handle* previewData);

/* Update edition preview */
OSErr UpdateEditionPreview(const EditionContainerSpec* container,
                          FormatType previewFormat,
                          const void* previewData,
                          Size dataSize);

/* Get preview thumbnail */
OSErr GetPreviewThumbnail(const EditionContainerSpec* container,
                         SInt32 thumbnailWidth,
                         SInt32 thumbnailHeight,
                         Handle* thumbnailData);

/*
 * Edition Import/Export
 */

/* Import edition from external format */
OSErr ImportEditionFromFile(const char* filePath,
                           FormatType sourceFormat,
                           EditionContainerSpec* newContainer);

/* Export edition to external format */
OSErr ExportEditionToFile(const EditionContainerSpec* container,
                         const char* outputPath,
                         FormatType targetFormat);

/* Batch import multiple files */
OSErr BatchImportEditions(const char** filePaths,
                         const FormatType* sourceFormats,
                         SInt32 fileCount,
                         EditionContainerSpec** newContainers,
                         SInt32* successCount);

/*
 * Platform-Specific UI Functions
 */

/* Show platform-specific subscriber dialog */
OSErr ShowPlatformSubscriberDialog(NewSubscriberReply* reply);

/* Show platform-specific publisher dialog */
OSErr ShowPlatformPublisherDialog(NewPublisherReply* reply);

/* Show platform-specific section options dialog */
OSErr ShowPlatformSectionOptionsDialog(SectionOptionsReply* reply);

/* Show platform-specific edition browser interface */
OSErr ShowEditionBrowserInterface(EditionContainerSpec* selectedContainer);

/*
 * Accessibility and Localization
 */

/* Set browser UI language */
OSErr SetBrowserLanguage(const char* languageCode);

/* Get localized string for browser UI */
OSErr GetLocalizedBrowserString(const char* stringKey,
                               char* localizedString,
                               Size stringSize);

/* Set accessibility options */

OSErr SetBrowserAccessibility(const AccessibilityOptions* options);

/*
 * Performance and Caching
 */

/* Enable/disable preview caching */
OSErr SetPreviewCaching(Boolean enable, Size maxCacheSize);

/* Clear preview cache */
OSErr ClearPreviewCache(void);

/* Set scan performance options */

OSErr SetScanOptions(const ScanOptions* options);

/*
 * Events and Notifications
 */

/* Browser event types */

/* Browser event callback */

/* Set browser event callback */
OSErr SetBrowserEventCallback(BrowserEventCallback callback, void* userData);

/* Post browser event */
OSErr PostBrowserEvent(BrowserEventType eventType, void* eventData);

/*
 * Constants and Limits
 */

#define kMaxEditionsInBrowser 1000      /* Maximum editions to display */
#define kMaxRecentEditions 20           /* Maximum recent editions */
#define kMaxPreviewSize 32768           /* Maximum preview data size */
#define kDefaultThumbnailSize 64        /* Default thumbnail size */
#define kMaxSearchResults 500           /* Maximum search results */

/* Browser view modes */

/* Sort options */

/* Error codes specific to browser */

#ifdef __cplusplus
}
#endif

#endif /* __EDITION_BROWSER_H__ */
