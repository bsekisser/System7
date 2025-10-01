/*
 * HelpBalloons.h - Help Balloon Display and Positioning
 *
 * This file defines structures and functions for displaying and positioning
 * help balloons in the Mac OS Help Manager. Balloons are floating windows
 * that display context-sensitive help information.
 */

#ifndef HELPBALLOONS_H
#define HELPBALLOONS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "WindowManager/WindowTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "Windows.h"
#include "HelpManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Balloon positioning constants */

/* Balloon tail positions */

/* Balloon appearance styles */

/* Balloon animation types */

/* Balloon state information */

/* Balloon positioning parameters */

/* Balloon content rendering information */

/* Modern balloon configuration */

/* Balloon display functions */
OSErr HMBalloonCreate(const HMBalloonContent *content,
                     const HMBalloonPlacement *placement,
                     HMBalloonInfo *balloonInfo);

OSErr HMBalloonShow(HMBalloonInfo *balloonInfo, short method);
OSErr HMBalloonHide(HMBalloonInfo *balloonInfo);
OSErr HMBalloonUpdate(HMBalloonInfo *balloonInfo,
                     const HMBalloonContent *newContent);

/* Balloon positioning functions */
OSErr HMBalloonCalculateRect(const HMBalloonContent *content,
                           const HMBalloonPlacement *placement,
                           Rect *balloonRect, Point *tipPoint,
                           HMBalloonTailPosition *tailPos);

OSErr HMBalloonFindBestPosition(const Rect *hotRect, const Rect *screenBounds,
                              const Size *balloonSize, Point *tipPoint,
                              Rect *balloonRect, HMBalloonTailPosition *tailPos);

Boolean HMBalloonRectFitsOnScreen(const Rect *balloonRect, const Rect *screenBounds);
OSErr HMBalloonAdjustForScreen(Rect *balloonRect, const Rect *screenBounds);

/* Balloon drawing functions */
OSErr HMBalloonDrawBackground(const Rect *balloonRect,
                            HMBalloonTailPosition tailPos,
                            const HMBalloonContent *content);

OSErr HMBalloonDrawContent(const Rect *contentRect,
                         const HMBalloonContent *content);

OSErr HMBalloonDrawBorder(const Rect *balloonRect,
                        HMBalloonTailPosition tailPos,
                        const HMBalloonContent *content);

OSErr HMBalloonDrawTail(const Rect *balloonRect, Point tipPoint,
                      HMBalloonTailPosition tailPos,
                      const HMBalloonContent *content);

/* Balloon region functions */
OSErr HMBalloonCreateRegions(const Rect *balloonRect, Point tipPoint,
                           HMBalloonTailPosition tailPos,
                           RgnHandle balloonRgn, RgnHandle structRgn);

OSErr HMBalloonCreateContentRgn(const Rect *balloonRect, RgnHandle contentRgn);
OSErr HMBalloonCreateTailRgn(const Rect *balloonRect, Point tipPoint,
                           HMBalloonTailPosition tailPos, RgnHandle tailRgn);

/* Balloon animation functions */
OSErr HMBalloonAnimate(HMBalloonInfo *balloonInfo, HMBalloonAnimation animation,
                     Boolean showing, long duration);

OSErr HMBalloonFadeIn(HMBalloonInfo *balloonInfo, long duration);
OSErr HMBalloonFadeOut(HMBalloonInfo *balloonInfo, long duration);
OSErr HMBalloonSlideIn(HMBalloonInfo *balloonInfo, HMBalloonTailPosition fromSide,
                     long duration);
OSErr HMBalloonSlideOut(HMBalloonInfo *balloonInfo, HMBalloonTailPosition toSide,
                      long duration);

/* Balloon utilities */
Boolean HMBalloonHitTest(const HMBalloonInfo *balloonInfo, Point mousePoint);
OSErr HMBalloonInvalidate(const HMBalloonInfo *balloonInfo);
OSErr HMBalloonGetWindow(const HMBalloonInfo *balloonInfo, WindowPtr *window);

/* Modern balloon functions */
OSErr HMBalloonConfigureModern(const HMModernBalloonConfig *config);
OSErr HMBalloonSetStyle(HMBalloonInfo *balloonInfo, HMBalloonStyle style);
OSErr HMBalloonSetAnimation(HMBalloonInfo *balloonInfo, HMBalloonAnimation animation);
OSErr HMBalloonSetOpacity(HMBalloonInfo *balloonInfo, float opacity);

/* Accessibility support */
OSErr HMBalloonSetAccessibilityDescription(HMBalloonInfo *balloonInfo,
                                         const char *description);
OSErr HMBalloonAnnounceToScreenReader(const HMBalloonInfo *balloonInfo);
Boolean HMBalloonIsAccessibilityEnabled(void);

/* Balloon management */
OSErr HMBalloonManagerInit(void);
void HMBalloonManagerShutdown(void);
OSErr HMBalloonGetCurrent(HMBalloonInfo **currentBalloon);
OSErr HMBalloonSetCurrent(HMBalloonInfo *balloonInfo);
void HMBalloonCleanupAll(void);

/* Content measurement functions */
OSErr HMBalloonMeasureContent(const HMBalloonContent *content, Size *contentSize);
OSErr HMBalloonMeasureText(const char *text, short fontID, short fontSize,
                         short fontStyle, Size *textSize);
OSErr HMBalloonMeasurePicture(PicHandle picture, Size *pictureSize);

/* Platform-specific balloon rendering */
#ifdef PLATFORM_REMOVED_APPLE
OSErr HMBalloonDrawCocoa(const HMBalloonInfo *balloonInfo);
#endif

#ifdef PLATFORM_REMOVED_WIN32
OSErr HMBalloonDrawWin32(const HMBalloonInfo *balloonInfo);
#endif

#ifdef PLATFORM_REMOVED_LINUX
OSErr HMBalloonDrawGTK(const HMBalloonInfo *balloonInfo);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HELPBALLOONS_H */