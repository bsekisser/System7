/*
 * EditionManager.h
 *
 * Main Edition Manager API - System 7 Publish/Subscribe Functionality
 * Provides live data sharing and linking between applications for dynamic collaboration
 *
 * Derived from ROM Edition Manager analysis
 * This is a portable implementation that abstracts the original Mac OS functionality
 */

#ifndef __EDITION_MANAGER_H__
#define __EDITION_MANAGER_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

/* Forward declarations */

/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Basic types compatible with original Mac OS API */
/* OSType is defined in MacTypes.h */
/* OSErr is defined in MacTypes.h */

/* Size is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* Str31 is defined in MacTypes.h */

/* Error codes */

/* Section types */
typedef OSType SectionType;
typedef OSType FormatType;
typedef SInt16 UpdateMode;

/* File specification structure */
#ifndef FILESYSTEM_TYPES_DEFINED
#define FILESYSTEM_TYPES_DEFINED

#endif /* FILESYSTEM_TYPES_DEFINED */

/* Edition container specification */

/* Section record - core data structure for publishers and subscribers */

/* Edition information record */

/* Dialog reply structures */

/* Format I/O structures */

/* Edition opener structures */

/* Function pointer types */

/* Constants */

/* Resource types */
#define rSectionType 'sect'
#define kPublisherDocAliasFormat 'alis'
#define kPreviewFormat 'prvw'
#define kFormatListFormat 'fmts'

/* Format masks */

/* Finder types for edition files */
#define kPICTEditionFileType 'edtp'
#define kTEXTEditionFileType 'edtt'
#define ksndEditionFileType 'edts'
#define kUnknownEditionFileType 'edtu'

/* Section event constants */
#define sectionEventMsgClass 'sect'
#define sectionReadMsgID 'read'
#define sectionWriteMsgID 'writ'
#define sectionScrollMsgID 'scrl'
#define sectionCancelMsgID 'cncl'

#ifdef __cplusplus
extern "C" {
#endif

/* Core Edition Manager API */

/* Initialization and cleanup */
OSErr InitEditionPack(void);
OSErr QuitEditionPack(void);

/* Section management */
OSErr NewSection(const EditionContainerSpec* container,
                 const FSSpec* sectionDocument,
                 SectionType kind,
                 SInt32 sectionID,
                 UpdateMode initialMode,
                 SectionHandle* sectionH);

OSErr RegisterSection(const FSSpec* sectionDocument,
                     SectionHandle sectionH,
                     Boolean* aliasWasUpdated);

OSErr UnRegisterSection(SectionHandle sectionH);
OSErr IsRegisteredSection(SectionHandle sectionH);

OSErr AssociateSection(SectionHandle sectionH,
                      const FSSpec* newSectionDocument);

/* Edition container management */
OSErr CreateEditionContainerFile(const FSSpec* editionFile,
                                OSType fdCreator,
                                ScriptCode editionFileNameScript);

OSErr DeleteEditionContainerFile(const FSSpec* editionFile);

/* Edition I/O */
OSErr OpenEdition(SectionHandle subscriberSectionH,
                 EditionRefNum* refNum);

OSErr OpenNewEdition(SectionHandle publisherSectionH,
                    OSType fdCreator,
                    const FSSpec* publisherSectionDocument,
                    EditionRefNum* refNum);

OSErr CloseEdition(EditionRefNum whichEdition, Boolean successful);

/* Data format operations */
OSErr EditionHasFormat(EditionRefNum whichEdition,
                      FormatType whichFormat,
                      Size* formatSize);

OSErr ReadEdition(EditionRefNum whichEdition,
                 FormatType whichFormat,
                 void* buffPtr,
                 Size* buffLen);

OSErr WriteEdition(EditionRefNum whichEdition,
                  FormatType whichFormat,
                  const void* buffPtr,
                  Size buffLen);

OSErr GetEditionFormatMark(EditionRefNum whichEdition,
                          FormatType whichFormat,
                          UInt32* currentMark);

OSErr SetEditionFormatMark(EditionRefNum whichEdition,
                          FormatType whichFormat,
                          UInt32 setMarkTo);

/* Edition information */
OSErr GetEditionInfo(const SectionHandle sectionH,
                    EditionInfoRecord* editionInfo);

OSErr GoToPublisherSection(const EditionContainerSpec* container);

OSErr GetLastEditionContainerUsed(EditionContainerSpec* container);

OSErr GetStandardFormats(const EditionContainerSpec* container,
                        FormatType* previewFormat,
                        Handle preview,
                        Handle publisherAlias,
                        Handle formats);

/* Edition opener management */
OSErr GetEditionOpenerProc(EditionOpenerProcPtr* opener);
OSErr SetEditionOpenerProc(EditionOpenerProcPtr opener);

OSErr CallEditionOpenerProc(EditionOpenerVerb selector,
                           EditionOpenerParamBlock* PB,
                           EditionOpenerProcPtr routine);

OSErr CallFormatIOProc(FormatIOVerb selector,
                      FormatIOParamBlock* PB,
                      FormatIOProcPtr routine);

/* User interface dialogs */
OSErr NewSubscriberDialog(NewSubscriberReply* reply);
OSErr NewPublisherDialog(NewPublisherReply* reply);
OSErr SectionOptionsDialog(SectionOptionsReply* reply);

/* Private/internal functions */
OSErr GetCurrentAppRefNum(AppRefNum* thisApp);
OSErr PostSectionEvent(SectionHandle sectionH, AppRefNum toApp, ResType classID);
OSErr EditionBackGroundTask(void);

#ifdef __cplusplus
}
#endif

#endif /* __EDITION_MANAGER_H__ */
