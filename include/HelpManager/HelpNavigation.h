/*
 * HelpNavigation.h - Help Navigation and Cross-References
 *
 * This file defines structures and functions for help navigation, hyperlinks,
 * cross-references, and help topic hierarchies.
 */

#ifndef HELPNAVIGATION_H
#define HELPNAVIGATION_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "HelpManager.h"
#include "HelpContent.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Navigation link types */

/* Navigation action types */

/* Navigation link structure */

/* Help topic hierarchy node */

/* Navigation history entry */

/* Navigation bookmark */

/* Navigation state */

/* Link callback function */

/* Topic provider callback */

/* Navigation initialization */
OSErr HMNavInit(short maxHistoryEntries);
void HMNavShutdown(void);

/* Navigation state management */
OSErr HMNavGetState(HMNavState **state);
OSErr HMNavSaveState(void);
OSErr HMNavRestoreState(void);
OSErr HMNavResetState(void);

/* Topic hierarchy management */
OSErr HMNavCreateNode(const char *topicID, const char *title,
                     ResType resourceType, short resourceID,
                     HMNavNode **node);

OSErr HMNavAddChild(HMNavNode *parent, HMNavNode *child);
OSErr HMNavRemoveChild(HMNavNode *parent, HMNavNode *child);
OSErr HMNavMoveNode(HMNavNode *node, HMNavNode *newParent);

OSErr HMNavFindNode(const char *topicID, HMNavNode **node);
OSErr HMNavFindNodeByResource(ResType resourceType, short resourceID,
                            HMNavNode **node);

OSErr HMNavGetRoot(HMNavNode **rootNode);
OSErr HMNavSetRoot(HMNavNode *rootNode);

/* Navigation traversal */
OSErr HMNavGoToTopic(const char *topicID);
OSErr HMNavGoToResource(ResType resourceType, short resourceID);
OSErr HMNavGoToParent(void);
OSErr HMNavGoToChild(short childIndex);
OSErr HMNavGoToSibling(short siblingIndex);

OSErr HMNavGetCurrent(HMNavNode **currentNode);
OSErr HMNavGetParent(HMNavNode **parentNode);
OSErr HMNavGetChildren(HMNavNode ***children, short *childCount);
OSErr HMNavGetSiblings(HMNavNode ***siblings, short *siblingCount);

/* Link management */
OSErr HMNavAddLink(HMNavNode *fromNode, const HMNavLink *link);
OSErr HMNavRemoveLink(HMNavNode *fromNode, const char *linkID);
OSErr HMNavGetLinks(HMNavNode *node, HMNavLink ***links, short *linkCount);
OSErr HMNavFindLink(HMNavNode *node, const char *linkID, HMNavLink **link);

OSErr HMNavFollowLink(const HMNavLink *link);
OSErr HMNavRegisterLinkCallback(HMNavLinkType linkType, HMNavLinkCallback callback);

/* History management */
OSErr HMNavHistoryAdd(const char *topicID, ResType resourceType, short resourceID);
OSErr HMNavHistoryBack(void);
OSErr HMNavHistoryForward(void);
OSErr HMNavHistoryGoTo(short historyIndex);

OSErr HMNavHistoryGetCurrent(HMNavHistoryEntry **entry);
OSErr HMNavHistoryGetList(HMNavHistoryEntry ***entries, short *entryCount);
OSErr HMNavHistoryClear(void);

Boolean HMNavHistoryCanGoBack(void);
Boolean HMNavHistoryCanGoForward(void);

/* Bookmark management */
OSErr HMNavBookmarkAdd(const char *bookmarkName, const char *topicID,
                      ResType resourceType, short resourceID);
OSErr HMNavBookmarkRemove(const char *bookmarkID);
OSErr HMNavBookmarkGoTo(const char *bookmarkID);

OSErr HMNavBookmarkGetList(HMNavBookmark ***bookmarks, short *bookmarkCount);
OSErr HMNavBookmarkFind(const char *bookmarkID, HMNavBookmark **bookmark);
OSErr HMNavBookmarkRename(const char *bookmarkID, const char *newName);

/* Topic search and indexing */
OSErr HMNavSearchTopics(const char *searchText, HMNavNode ***results, short *resultCount);
OSErr HMNavSearchContent(const char *searchText, char ***results, short *resultCount);
OSErr HMNavBuildIndex(void);
OSErr HMNavGetIndex(char ***indexEntries, short *entryCount);

/* Cross-reference management */
OSErr HMNavAddCrossRef(const char *fromTopic, const char *toTopic,
                      const char *linkText);
OSErr HMNavRemoveCrossRef(const char *fromTopic, const char *toTopic);
OSErr HMNavGetCrossRefs(const char *topicID, HMNavLink ***crossRefs, short *refCount);

/* Topic validation */
Boolean HMNavValidateNode(const HMNavNode *node);
Boolean HMNavValidateHierarchy(const HMNavNode *rootNode);
OSErr HMNavRepairHierarchy(HMNavNode *rootNode);

/* Navigation preferences */
OSErr HMNavSetPreference(const char *prefName, const char *prefValue);
OSErr HMNavGetPreference(const char *prefName, char *prefValue, short maxLength);

OSErr HMNavSetHistorySize(short maxEntries);
short HMNavGetHistorySize(void);

OSErr HMNavSetTrackingEnabled(Boolean enabled);
Boolean HMNavGetTrackingEnabled(void);

/* Topic loading and caching */
OSErr HMNavPreloadTopic(const char *topicID);
OSErr HMNavPreloadChildren(const char *topicID);
OSErr HMNavFlushCache(void);

OSErr HMNavRegisterTopicProvider(HMNavTopicProvider provider);
OSErr HMNavLoadTopicOnDemand(const char *topicID, HMNavNode **node);

/* Navigation UI support */
OSErr HMNavGetBreadcrumb(char *breadcrumb, short maxLength);
OSErr HMNavGetNavigationMenu(MenuHandle *navMenu);
OSErr HMNavUpdateNavigationUI(void);

/* Export and import */
OSErr HMNavExportHierarchy(const char *filePath);
OSErr HMNavImportHierarchy(const char *filePath);
OSErr HMNavExportBookmarks(const char *filePath);
OSErr HMNavImportBookmarks(const char *filePath);

/* Modern navigation support */
OSErr HMNavSupportHyperlinks(Boolean enable);
OSErr HMNavSupportURLs(Boolean enable);
OSErr HMNavSetDefaultBrowser(const char *browserPath);

/* Accessibility navigation */
OSErr HMNavSetAccessibilityMode(Boolean enabled);
OSErr HMNavGetAccessibleDescription(const HMNavNode *node, char *description,
                                  short maxLength);
OSErr HMNavAnnounceNavigation(const HMNavNode *node);

/* Navigation debugging */
OSErr HMNavDumpHierarchy(void);
OSErr HMNavValidateNavigation(void);
OSErr HMNavGetStatistics(long *nodeCount, long *linkCount, long *historyCount,
                        long *bookmarkCount);

/* Node disposal */
void HMNavDisposeNode(HMNavNode *node);
void HMNavDisposeHierarchy(HMNavNode *rootNode);
void HMNavDisposeLink(HMNavLink *link);

#ifdef __cplusplus
}
#endif

#endif /* HELPNAVIGATION_H */