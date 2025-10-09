/* Control Manager Internal Functions */
#ifndef CONTROL_INTERNAL_H
#define CONTROL_INTERNAL_H

#include "SystemTypes.h"

/* Control tracking */
ControlHandle GetTrackingControl(void);
SInt16 GetTrackingPart(void);
Boolean IsControlTracking(ControlHandle control);

/* Standard controls */
void SetCheckboxMixed(ControlHandle ctrl, Boolean mixed);
Boolean GetCheckboxMixed(ControlHandle ctrl);
void SetRadioGroup(ControlHandle ctrl, SInt16 groupID);
SInt16 GetRadioGroup(ControlHandle ctrl);

/* Control stubs */
void GetWindowBounds(WindowPtr window, Rect* bounds);
void _AttachControlToWindow(ControlHandle ctrl, WindowPtr window);
ControlHandle LoadControlFromResource(SInt16 resID);

#endif /* CONTROL_INTERNAL_H */
