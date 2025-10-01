/*
 * Calculator Header - System 7.1 Calculator Desk Accessory
 * Portable implementation with cross-platform support
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "WindowManager/WindowTypes.h"
#include "EventManager/EventTypes.h"

/* Constants */
#define CALC_WINDOW_WIDTH    198
#define CALC_WINDOW_HEIGHT   192

#include "SystemTypes.h"
#define DISPLAY_HEIGHT       24

#include "SystemTypes.h"
#define BUTTON_WIDTH         30
#define BUTTON_HEIGHT        24

#include "SystemTypes.h"
#define BUTTON_SPACING       8
#define MAX_DIGITS           12

/* Button action codes */

/* Button structure */

/* Calculator state */

/* Function prototypes */
OSErr InitCalculator(void);
void HandleCalculatorEvent(EventRecord *event);
void CloseCalculator(void);
void HandleMouseDown(Point where);
short FindButton(Point pt);
void HandleKeyDown(char key);
void HandleUpdate(void);
void DrawCalculator(void);
void DrawDisplay(void);
void DrawButton(CalcButton *button);
void HandleButtonClick(short buttonIndex);
void HandleDigit(short digit);
void HandleOperator(short op);
void PerformCalculation(void);
void ClearCalculator(void);
void UpdateDisplay(void);
void SetError(const char *errorMsg);
double StringToDouble(const char *str);
void DoubleToString(double value, char *str);
WindowPtr GetCalculatorWindow(void);
Boolean IsCalculatorActive(void);

#endif /* CALCULATOR_H */