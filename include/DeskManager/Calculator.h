#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/*
 * Calculator.h - Calculator Desk Accessory
 *
 * Provides a complete calculator implementation with basic arithmetic,
 * scientific functions, and programmer operations. This matches the
 * functionality of the Mac OS Calculator desk accessory.
 *
 * Derived from ROM analysis (System 7)
 */

#include "DeskAccessory.h"

/* Calculator Constants */
#define CALC_VERSION            0x0200      /* Calculator version 2.0 */
#define CALC_DISPLAY_DIGITS     32          /* Maximum display digits */
#define CALC_MEMORY_SLOTS       10          /* Number of memory slots */
#define CALC_HISTORY_SIZE       20          /* Calculation history size */

#include "SystemTypes.h"

/* Calculator Modes */

/* Calculator Number Bases */

/* Calculator States */

/* Calculator Operations */

/* Calculator Button IDs (matching Mac OS Calculator) */

/* Calculator Number */

/* Calculator History Entry */

/* Calculator State */

/* Calculator Functions */

/**
 * Initialize calculator
 * @param calc Pointer to calculator structure
 * @return 0 on success, negative on error
 */
int Calculator_Initialize(Calculator *calc);

/**
 * Shutdown calculator
 * @param calc Pointer to calculator structure
 */
void Calculator_Shutdown(Calculator *calc);

/**
 * Reset calculator to initial state
 * @param calc Pointer to calculator structure
 */
void Calculator_Reset(Calculator *calc);

/**
 * Process button press
 * @param calc Pointer to calculator structure
 * @param buttonID Button ID
 * @return 0 on success, negative on error
 */
int Calculator_PressButton(Calculator *calc, CalcButtonID buttonID);

/**
 * Process keyboard input
 * @param calc Pointer to calculator structure
 * @param key Key character
 * @return 0 on success, negative on error
 */
int Calculator_KeyPress(Calculator *calc, char key);

/**
 * Perform calculation operation
 * @param calc Pointer to calculator structure
 * @param operation Operation to perform
 * @return 0 on success, negative on error
 */
int Calculator_PerformOperation(Calculator *calc, CalcOperation operation);

/**
 * Enter digit
 * @param calc Pointer to calculator structure
 * @param digit Digit to enter (0-15 for hex)
 * @return 0 on success, negative on error
 */
int Calculator_EnterDigit(Calculator *calc, int digit);

/**
 * Enter decimal point
 * @param calc Pointer to calculator structure
 * @return 0 on success, negative on error
 */
int Calculator_EnterDecimal(Calculator *calc);

/**
 * Clear current entry
 * @param calc Pointer to calculator structure
 */
void Calculator_Clear(Calculator *calc);

/**
 * Clear all (reset)
 * @param calc Pointer to calculator structure
 */
void Calculator_ClearAll(Calculator *calc);

/**
 * Backspace (remove last digit)
 * @param calc Pointer to calculator structure
 */
void Calculator_Backspace(Calculator *calc);

/* Mode and Base Functions */

/**
 * Set calculator mode
 * @param calc Pointer to calculator structure
 * @param mode New mode
 * @return 0 on success, negative on error
 */
int Calculator_SetMode(Calculator *calc, CalcMode mode);

/**
 * Set number base (programmer mode)
 * @param calc Pointer to calculator structure
 * @param base New base
 * @return 0 on success, negative on error
 */
int Calculator_SetBase(Calculator *calc, CalcBase base);

/**
 * Toggle angle mode (radians/degrees)
 * @param calc Pointer to calculator structure
 */
void Calculator_ToggleAngleMode(Calculator *calc);

/* Memory Functions */

/**
 * Clear memory slot
 * @param calc Pointer to calculator structure
 * @param slot Memory slot (0-9)
 */
void Calculator_MemoryClear(Calculator *calc, int slot);

/**
 * Store value in memory slot
 * @param calc Pointer to calculator structure
 * @param slot Memory slot (0-9)
 */
void Calculator_MemoryStore(Calculator *calc, int slot);

/**
 * Recall value from memory slot
 * @param calc Pointer to calculator structure
 * @param slot Memory slot (0-9)
 * @return 0 on success, negative on error
 */
int Calculator_MemoryRecall(Calculator *calc, int slot);

/**
 * Add to memory slot
 * @param calc Pointer to calculator structure
 * @param slot Memory slot (0-9)
 */
void Calculator_MemoryAdd(Calculator *calc, int slot);

/**
 * Subtract from memory slot
 * @param calc Pointer to calculator structure
 * @param slot Memory slot (0-9)
 */
void Calculator_MemorySubtract(Calculator *calc, int slot);

/* History Functions */

/**
 * Add calculation to history
 * @param calc Pointer to calculator structure
 * @param op1 First operand
 * @param op2 Second operand
 * @param operation Operation
 * @param result Result
 */
void Calculator_AddToHistory(Calculator *calc, const CalcNumber *op1,
                           const CalcNumber *op2, CalcOperation operation,
                           const CalcNumber *result);

/**
 * Get history entry
 * @param calc Pointer to calculator structure
 * @param index History index
 * @return Pointer to history entry or NULL
 */
const CalcHistoryEntry *Calculator_GetHistoryEntry(Calculator *calc, int index);

/**
 * Clear calculation history
 * @param calc Pointer to calculator structure
 */
void Calculator_ClearHistory(Calculator *calc);

/* Display Functions */

/**
 * Get display string
 * @param calc Pointer to calculator structure
 * @return Display string
 */
const char *Calculator_GetDisplay(Calculator *calc);

/**
 * Update display for current state
 * @param calc Pointer to calculator structure
 */
void Calculator_UpdateDisplay(Calculator *calc);

/**
 * Format number for display
 * @param number Pointer to number
 * @param buffer Buffer for formatted string
 * @param bufferSize Size of buffer
 */
void Calculator_FormatNumber(const CalcNumber *number, char *buffer, int bufferSize);

/* Desk Accessory Integration */

/**
 * Register Calculator as a desk accessory
 * @return 0 on success, negative on error
 */
int Calculator_RegisterDA(void);

/**
 * Create Calculator DA instance
 * @return Pointer to DA instance or NULL on error
 */
DeskAccessory *Calculator_CreateDA(void);

/* Calculator Error Codes */
#define CALC_ERR_NONE           0       /* No error */
#define CALC_ERR_DIVIDE_BY_ZERO -1      /* Division by zero */
#define CALC_ERR_OVERFLOW       -2      /* Numeric overflow */
#define CALC_ERR_UNDERFLOW      -3      /* Numeric underflow */
#define CALC_ERR_DOMAIN         -4      /* Domain error (e.g., sqrt(-1)) */
#define CALC_ERR_INVALID_OP     -5      /* Invalid operation */
#define CALC_ERR_INVALID_BASE   -6      /* Invalid number base */
#define CALC_ERR_MEMORY_EMPTY   -7      /* Memory slot empty */

#endif /* CALCULATOR_H */