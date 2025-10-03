/*
 * TextEditApp.c - TextEdit Application Entry Point
 *
 * Minimal System 7-faithful TextEdit application.
 * Relies on canonical GetNextEvent/DispatchEvent provided by the system.
 */

#include "SystemTypes.h"
#include "System71StdLib.h"
#include "TextEdit/TextEdit.h"
#include "WindowManager/WindowManager.h"
#include "EventManager/EventManager.h"
#include "QuickDraw/QuickDraw.h"
#include "FileManager.h"
#include "MemoryMgr/MemoryManager.h"
#include "Errors/ErrorCodes.h"
#include <string.h>

extern void serial_printf(const char* fmt, ...);

/* Application state */
static WindowPtr gTextEditWindow = NULL;
static TEHandle gTextEditTE = NULL;
static Boolean gAppRunning = false;

/*
 * TextEdit_InitApp - Initialize TextEdit application
 * Called by Process Manager when app launches
 */
OSErr TextEdit_InitApp(void)
{
    Rect windRect;
    Rect destRect;
    Rect viewRect;

    serial_printf("TextEdit_InitApp: Initializing TextEdit application\n");

    /* Create window */
    SetRect(&windRect, 100, 100, 500, 400);
    /* Create Pascal string for window title */
    static unsigned char titleStr[] = "\010TextEdit"; /* \010 = length 8 in octal */
    gTextEditWindow = NewWindow(NULL, &windRect, titleStr, true,
                                0 /* documentProc */, (WindowPtr)-1, true, 0);

    if (!gTextEditWindow) {
        serial_printf("TextEdit_InitApp: Failed to create window\n");
        return memFullErr;
    }

    /* Set port to window */
    SetPort(gTextEditWindow);

    /* Create TextEdit rect (inset from window content) */
    viewRect = gTextEditWindow->port.portRect;
    InsetRect(&viewRect, 4, 4);

    /* Initialize TextEdit handle */
    gTextEditTE = TENew(&viewRect, &viewRect);

    if (!gTextEditTE) {
        serial_printf("TextEdit_InitApp: Failed to create TE handle\n");
        DisposeWindow(gTextEditWindow);
        gTextEditWindow = NULL;
        return memFullErr;
    }

    /* Insert initial text */
    const char* initialText = "Welcome to TextEdit!\r\rThis is a minimal System 7-faithful text editor.";
    TEInsert(initialText, strlen(initialText), gTextEditTE);

    gAppRunning = true;

    serial_printf("TextEdit_InitApp: Application initialized successfully\n");
    return noErr;
}

/*
 * TextEdit_HandleEvent - Handle events for TextEdit
 * Called by Process Manager for this app's events
 */
void TextEdit_HandleEvent(EventRecord* event)
{
    if (!gAppRunning || !gTextEditWindow) {
        return;
    }

    switch (event->what) {
        case updateEvt:
            if ((WindowPtr)event->message == gTextEditWindow) {
                BeginUpdate(gTextEditWindow);
                SetPort(gTextEditWindow);
                EraseRect(&gTextEditWindow->port.portRect);
                TEUpdate(&gTextEditWindow->port.portRect, gTextEditTE);
                EndUpdate(gTextEditWindow);
            }
            break;

        case activateEvt:
            if ((WindowPtr)event->message == gTextEditWindow) {
                if (event->modifiers & activeFlag) {
                    TEActivate(gTextEditTE);
                } else {
                    TEDeactivate(gTextEditTE);
                }
            }
            break;

        case keyDown:
        case autoKey:
            if (FrontWindow() == gTextEditWindow) {
                char key = (char)(event->message & charCodeMask);
                TEKey(key, gTextEditTE);
            }
            break;

        case mouseDown:
            {
                WindowPtr whichWindow;
                short part = FindWindow(event->where, &whichWindow);

                if (part == inContent && whichWindow == gTextEditWindow) {
                    /* Convert to local coordinates */
                    Point localPt = event->where;
                    SetPort(gTextEditWindow);
                    GlobalToLocal(&localPt);
                    TEClick(localPt, (event->modifiers & shiftKey) != 0, gTextEditTE);
                }
            }
            break;
    }
}

/*
 * TextEdit_CleanupApp - Cleanup on quit
 */
void TextEdit_CleanupApp(void)
{
    if (gTextEditTE) {
        TEDispose(gTextEditTE);
        gTextEditTE = NULL;
    }

    if (gTextEditWindow) {
        DisposeWindow(gTextEditWindow);
        gTextEditWindow = NULL;
    }

    gAppRunning = false;

    serial_printf("TextEdit_CleanupApp: Application cleaned up\n");
}

/*
 * TextEdit_IsRunning - Check if app is running
 */
Boolean TextEdit_IsRunning(void)
{
    return gAppRunning;
}

/*
 * TextEdit_LoadFile - Load a file into TextEdit window
 * fileName: Pascal string (length-prefixed) filename
 * vRefNum: Volume reference number (0 for default volume)
 */
OSErr TextEdit_LoadFile(ConstStr255Param fileName, VolumeRefNum vRefNum)
{
    FileRefNum refNum;
    OSErr err;
    UInt32 fileSize;
    char* buffer = NULL;

    serial_printf("TextEdit_LoadFile: Loading file \"%.*s\"\n", fileName[0], fileName+1);

    /* Make sure TextEdit is initialized */
    if (!gAppRunning || !gTextEditWindow || !gTextEditTE) {
        serial_printf("TextEdit_LoadFile: TextEdit not initialized\n");
        return fnfErr;
    }

    /* Open the file */
    err = FSOpen(fileName, vRefNum, &refNum);
    if (err != noErr) {
        serial_printf("TextEdit_LoadFile: FSOpen failed, err=%d\n", err);
        return err;
    }

    /* Get file size */
    err = FSGetEOF(refNum, &fileSize);
    if (err != noErr) {
        serial_printf("TextEdit_LoadFile: FSGetEOF failed, err=%d\n", err);
        FSClose(refNum);
        return err;
    }

    serial_printf("TextEdit_LoadFile: File size = %u bytes\n", fileSize);

    /* Allocate buffer for file content */
    if (fileSize > 0) {
        buffer = (char*)NewPtr(fileSize + 1);
        if (!buffer) {
            serial_printf("TextEdit_LoadFile: Failed to allocate buffer\n");
            FSClose(refNum);
            return memFullErr;
        }

        /* Read file content */
        UInt32 bytesToRead = fileSize;
        err = FSRead(refNum, &bytesToRead, buffer);
        if (err != noErr && err != eofErr) {
            serial_printf("TextEdit_LoadFile: FSRead failed, err=%d\n", err);
            DisposePtr((Ptr)buffer);
            FSClose(refNum);
            return err;
        }

        buffer[bytesToRead] = '\0';
        serial_printf("TextEdit_LoadFile: Read %u bytes\n", bytesToRead);

        /* Clear existing text and insert file content */
        SetPort(gTextEditWindow);
        TESetSelect(0, 32767, gTextEditTE);  /* Select all */
        TEDelete(gTextEditTE);                 /* Delete existing text */
        TEInsert(buffer, bytesToRead, gTextEditTE);

        DisposePtr((Ptr)buffer);
    }

    /* Close file */
    FSClose(refNum);

    /* Update window title with filename */
    SetWTitle(gTextEditWindow, fileName);

    /* Refresh window */
    InvalRect(&gTextEditWindow->port.portRect);

    serial_printf("TextEdit_LoadFile: File loaded successfully\n");
    return noErr;
}
