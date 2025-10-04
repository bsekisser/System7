/*
 * textedit_stubs.c - Temporary TextEdit stubs
 */

#include "SystemTypes.h"
#include "TextEdit/TextEdit.h"

/* Forward declarations */
void TextEdit_InitApp(void);
Boolean TextEdit_IsRunning(void);
void TextEdit_HandleEvent(void* event);

/* TextEdit stubs to allow compilation */
void TextEdit_InitApp(void) {
    /* Stub */
}

Boolean TextEdit_IsRunning(void) {
    return 0;
}

void TextEdit_HandleEvent(void* event) {
    /* Stub */
}

void TextEdit_LoadFile(const char* path) {
    /* Stub */
}

/* Basic TextEdit stubs */
void TEInit(void) {
    /* Stub */
}

TEHandle TENew(const Rect *destRect, const Rect *viewRect) {
    return NULL;
}

void TEDispose(TEHandle hTE) {
    /* Stub */
}

void TESetText(const void *text, SInt32 length, TEHandle hTE) {
    /* Stub */
}

void TEKey(CharParameter key, TEHandle hTE) {
    /* Stub */
}

void TEClick(Point pt, Boolean extend, TEHandle hTE) {
    /* Stub */
}

void TEUpdate(const Rect *updateRect, TEHandle hTE) {
    /* Stub */
}

void TEActivate(TEHandle hTE) {
    /* Stub */
}

void TEDeactivate(TEHandle hTE) {
    /* Stub */
}

void TEIdle(TEHandle hTE) {
    /* Stub */
}