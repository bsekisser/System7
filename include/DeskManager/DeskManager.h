#ifndef DESKMANAGER_H
#define DESKMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/*
 * DeskManager.h - Main Desk Manager API
 *
 * Provides the core API for managing desk accessories (DAs) in System 7.1.
 * Desk accessories are small utility programs that provide essential daily
 * computing functions like Calculator, Key Caps, Alarm Clock, and Chooser.
 *
 * Based on Apple's Macintosh Toolbox Desk Manager from System 7.1
 */

/* Forward declarations */

/* Desk Manager Constants */
#define DESK_MGR_VERSION        0x0701      /* System 7.1 */
#define MAX_DESK_ACCESSORIES    64          /* Maximum concurrent DAs */
#define DA_NAME_LENGTH          255         /* Maximum DA name length */

/* Desk Accessory Messages */

/* Desk Accessory Types */

/* Desk Accessory States */

/* Desk Accessory Control Block */

/* Desk Manager State */

/* Core Desk Manager Functions */

/**
 * Initialize the Desk Manager
 * @return 0 on success, negative on error
 */
int DeskManager_Initialize(void);

/**
 * Shutdown the Desk Manager
 */
void DeskManager_Shutdown(void);

/**
 * Open a desk accessory by name
 * @param name Name of the desk accessory
 * @return Reference number on success, negative on error
 */
SInt16 OpenDeskAcc(const char *name);

/**
 * Close a desk accessory
 * @param refNum Reference number of the DA to close
 */
void CloseDeskAcc(SInt16 refNum);

/**
 * Handle system-level events for desk accessories
 * @param event Pointer to event record
 * @return true if event was handled by a DA
 */
Boolean SystemEvent(const EventRecord *event);

/**
 * Handle mouse clicks in system areas
 * @param event Pointer to event record
 * @param window Window where click occurred
 */
void SystemClick(const EventRecord *event, WindowRecord *window);

/**
 * Perform periodic processing for all DAs
 */
void SystemTask(void);

/**
 * Handle system menu selections
 * @param menuResult Menu selection result
 */
void SystemMenu(SInt32 menuResult);

/**
 * Handle system edit operations
 * @param editCmd Edit command (cut, copy, paste, etc.)
 * @return true if command was handled
 */
Boolean SystemEdit(SInt16 editCmd);

/* Desk Accessory Management Functions */

/**
 * Get desk accessory by reference number
 * @param refNum Reference number
 * @return Pointer to DA or NULL if not found
 */
DeskAccessory *DA_GetByRefNum(SInt16 refNum);

/**
 * Get desk accessory by name
 * @param name DA name
 * @return Pointer to DA or NULL if not found
 */
DeskAccessory *DA_GetByName(const char *name);

/**
 * Get currently active desk accessory
 * @return Pointer to active DA or NULL if none
 */
DeskAccessory *DA_GetActive(void);

/**
 * Set active desk accessory
 * @param da Pointer to DA to activate
 * @return 0 on success, negative on error
 */
int DA_SetActive(DeskAccessory *da);

/**
 * Send message to desk accessory
 * @param da Pointer to DA
 * @param message Message type
 * @param param1 First parameter
 * @param param2 Second parameter
 * @return Result from DA
 */
int DA_SendMessage(DeskAccessory *da, DAMessage message,
                   void *param1, void *param2);

/* System Integration Functions */

/**
 * Add DA to system menu (Apple menu)
 * @param da Pointer to DA
 * @return 0 on success, negative on error
 */
int SystemMenu_AddDA(DeskAccessory *da);

/**
 * Remove DA from system menu
 * @param da Pointer to DA
 */
void SystemMenu_RemoveDA(DeskAccessory *da);

/**
 * Update system menu
 */
void SystemMenu_Update(void);

/* Utility Functions */

/**
 * Get Desk Manager version
 * @return Version number (0x0701 for System 7.1)
 */
UInt16 DeskManager_GetVersion(void);

/**
 * Get number of open desk accessories
 * @return Number of open DAs
 */
SInt16 DeskManager_GetDACount(void);

/**
 * Check if a DA is installed
 * @param name DA name
 * @return true if DA is available
 */
Boolean DeskManager_IsDAAvailable(const char *name);

/* Built-in Desk Accessories */

/**
 * Register built-in desk accessories
 * @return 0 on success, negative on error
 */
int DeskManager_RegisterBuiltinDAs(void);

/* Error Codes */
#define DESK_ERR_NONE           0       /* No error */
#define DESK_ERR_NO_MEMORY      -1      /* Out of memory */
#define DESK_ERR_NOT_FOUND      -2      /* DA not found */
#define DESK_ERR_ALREADY_OPEN   -3      /* DA already open */
#define DESK_ERR_INVALID_PARAM  -4      /* Invalid parameter */
#define DESK_ERR_SYSTEM_ERROR   -5      /* System error */
#define DESK_ERR_DA_ERROR       -6      /* DA-specific error */

#endif /* DESKMANAGER_H */