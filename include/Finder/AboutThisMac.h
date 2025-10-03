/*
 * AboutThisMac.h - System 7.1-style "About This Macintosh" Window
 *
 * Public API for the About window.
 */

#ifndef ABOUT_THIS_MAC_H
#define ABOUT_THIS_MAC_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AboutWindow_ShowOrToggle - Show About window or bring to front
 *
 * Called from Apple menu handler. Creates window if needed,
 * otherwise brings existing window to front.
 */
void AboutWindow_ShowOrToggle(void);

/*
 * AboutWindow_CloseIf - Close About window if it matches
 *
 * Parameters:
 *   w - Window to check and potentially close
 */
void AboutWindow_CloseIf(WindowPtr w);

/*
 * AboutWindow_HandleUpdate - Handle update event for About window
 *
 * Parameters:
 *   w - Window receiving update event
 *
 * Returns:
 *   true if this was the About window and update was handled
 */
Boolean AboutWindow_HandleUpdate(WindowPtr w);

/*
 * AboutWindow_HandleMouseDown - Handle mouse down in About window
 *
 * Parameters:
 *   w - Window receiving mouse down
 *   part - Window part code (inDrag, inGoAway, etc.)
 *   localPt - Mouse location in local coordinates
 *
 * Returns:
 *   true if this was the About window and event was handled
 */
Boolean AboutWindow_HandleMouseDown(WindowPtr w, short part, Point localPt);

/*
 * AboutWindow_IsOurs - Check if window is the About window
 *
 * Parameters:
 *   w - Window to check
 *
 * Returns:
 *   true if window is the About This Macintosh window
 */
Boolean AboutWindow_IsOurs(WindowPtr w);

#ifdef __cplusplus
}
#endif

#endif /* ABOUT_THIS_MAC_H */
