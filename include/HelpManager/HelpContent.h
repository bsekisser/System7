/*
 * HelpContent.h - Help Content Loading and Formatting
 *
 * This file defines structures and functions for loading, formatting, and
 * managing help content in various formats including text, pictures, and
 * styled text.
 */

#ifndef HELPCONTENT_H
#define HELPCONTENT_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "ResourceManager.h"
#include "TextEdit.h"
#include "HelpManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Content format types */

/* Content source types */

/* Content loading options */

/* Content metadata */

/* Styled text information */

/* Picture content information */

/* Mixed content element */

        struct {
            PicHandle picture;       /* Picture handle */
            Rect bounds;             /* Picture bounds */
            char altText[256];       /* Alternative text */
        } picture;
        struct {
            char *htmlContent;       /* HTML content */
            char baseURL[256];       /* Base URL for relative links */
        } html;
    } content;
    struct HMContentElement *next;   /* Next element in list */
} HMContentElement;

/* Content container */

    Boolean isLoaded;                /* Content loaded flag */
    long loadTime;                   /* Time content was loaded */
    OSErr lastError;                 /* Last loading error */
} HMContentContainer;

/* Content cache entry */

/* Content loading callback */

/* Content processing callback */

/* Content loading functions */
OSErr HMContentLoad(const HMContentOptions *options, const char *contentSpec,
                   HMContentContainer **container);

OSErr HMContentLoadFromResource(ResType resourceType, short resourceID,
                              short index, HMContentContainer **container);

OSErr HMContentLoadFromFile(const char *filePath, HMContentFormat format,
                          HMContentContainer **container);

OSErr HMContentLoadFromURL(const char *url, HMContentContainer **container);

OSErr HMContentLoadFromMemory(const void *data, long dataSize,
                            HMContentFormat format, HMContentContainer **container);

/* Content formatting functions */
OSErr HMContentFormat(HMContentContainer *container, HMContentFormat targetFormat);

OSErr HMContentFormatText(const char *plainText, const HMContentOptions *options,
                        HMStyledTextInfo *styledText);

OSErr HMContentFormatHTML(const char *htmlContent, HMStyledTextInfo *styledText);

OSErr HMContentFormatMarkdown(const char *markdownContent, HMStyledTextInfo *styledText);

/* Content measurement functions */
OSErr HMContentMeasure(const HMContentContainer *container, Size *contentSize);

OSErr HMContentMeasureText(const HMStyledTextInfo *styledText, Size *textSize);

OSErr HMContentMeasurePicture(const HMPictureInfo *picture, Size *pictureSize);

/* Content rendering functions */
OSErr HMContentRender(const HMContentContainer *container, const Rect *renderRect,
                     short renderMode);

OSErr HMContentRenderText(const HMStyledTextInfo *styledText, const Rect *textRect);

OSErr HMContentRenderPicture(const HMPictureInfo *picture, const Rect *pictureRect);

OSErr HMContentRenderMixed(const HMContentElement *elements, const Rect *renderRect);

/* Content cache management */
OSErr HMContentCacheInit(short maxEntries, long maxMemory);
void HMContentCacheShutdown(void);

OSErr HMContentCacheStore(const char *cacheKey, HMContentContainer *container);
OSErr HMContentCacheRetrieve(const char *cacheKey, HMContentContainer **container);
OSErr HMContentCacheRemove(const char *cacheKey);
void HMContentCacheClear(void);

OSErr HMContentCacheGetStatistics(short *entryCount, long *memoryUsed,
                                 long *hitCount, long *missCount);

/* Content validation functions */
Boolean HMContentValidate(const HMContentContainer *container);
OSErr HMContentCheckFormat(const void *data, long dataSize, HMContentFormat *format);
Boolean HMContentIsAccessible(const HMContentContainer *container);

/* Content conversion functions */
OSErr HMContentConvertToPlainText(const HMContentContainer *source, char **plainText);
OSErr HMContentConvertToHTML(const HMContentContainer *source, char **htmlContent);
OSErr HMContentConvertToAccessibleText(const HMContentContainer *source, char **accessibleText);

/* String resource utilities */
OSErr HMContentLoadStringResource(short resourceID, short index, char **string);
OSErr HMContentLoadStringList(short resourceID, Handle *stringListHandle);
OSErr HMContentGetStringFromList(Handle stringListHandle, short index, char **string);

/* Picture resource utilities */
OSErr HMContentLoadPictureResource(short resourceID, PicHandle *picture);
OSErr HMContentScalePicture(PicHandle sourcePicture, const Size *targetSize,
                          PicHandle *scaledPicture);
OSErr HMContentConvertPictureFormat(PicHandle sourcePicture, short targetFormat,
                                  Handle *convertedPicture);

/* Styled text utilities */
OSErr HMContentCreateStyledText(const char *plainText, const HMContentOptions *options,
                              TEHandle *styledTextHandle);
OSErr HMContentApplyTextStyle(TEHandle textHandle, short startOffset, short length,
                            short fontID, short fontSize, short fontStyle,
                            const RGBColor *color);
OSErr HMContentInsertHyperlink(TEHandle textHandle, short startOffset, short length,
                             const char *linkTarget);

/* Modern content support */
OSErr HMContentSupportUTF8(Boolean enable);
OSErr HMContentSetDefaultEncoding(const char *encoding);
OSErr HMContentRegisterLoader(HMContentFormat format, HMContentLoadCallback callback);
OSErr HMContentRegisterProcessor(HMContentFormat format, HMContentProcessCallback callback);

/* Accessibility content functions */
OSErr HMContentGenerateAccessibleDescription(const HMContentContainer *container,
                                           char **description);
OSErr HMContentGenerateScreenReaderText(const HMContentContainer *container,
                                       char **screenReaderText);
Boolean HMContentHasAccessibleAlternative(const HMContentContainer *container);

/* Content disposal functions */
void HMContentDispose(HMContentContainer *container);
void HMContentDisposeElement(HMContentElement *element);
void HMContentDisposeStyledText(HMStyledTextInfo *styledText);
void HMContentDisposePicture(HMPictureInfo *picture);

#ifdef __cplusplus
}
#endif

#endif /* HELPCONTENT_H */