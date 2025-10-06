#ifndef EVENTMANAGERINTERNAL_H
#define EVENTMANAGERINTERNAL_H

#include "../SystemTypes.h"
#include "EventManager.h"

// Internal event structures
typedef struct EventQueueEntry {
    EventRecord event;
    struct EventQueueEntry* next;
} EventQueueEntry;

typedef struct EventManagerState {
    EventQueueEntry* eventQueue;
    EventQueueEntry* eventQueueTail;
    short queueSize;
    Boolean mouseDown;
    Point lastMousePos;
    UInt32 lastClickTime;
} EventManagerState;

// Internal functions
void InitEventInternals(void);
Boolean QueueEvent(EventRecord* event);
Boolean DequeueEvent(EventRecord* event);

// Event Dispatcher
void InitEventDispatcher(void);
Boolean DispatchEvent(EventRecord* evt);
WindowPtr GetActiveWindow(void);
void SetActiveWindow(WindowPtr window);

// Modern Input
void UpdateMouseState(void);
void UpdateKeyboardState(void);
Boolean IsModernInputInitialized(void);
const char* GetModernInputPlatform(void);

// System Events
SInt16 GenerateSystemEventEx(EventKind what, WindowPtr window, Point globalWhere, UInt32 message, UInt16 modifiers);

// Process Manager Event Integration
OSErr Proc_PostEvent(EventKind what, UInt32 message);
Boolean Proc_GetNextEvent(EventMask eventMask, EventRecord* theEvent);
Boolean Proc_EventAvail(EventMask eventMask, EventRecord* theEvent);
void Event_InitQueue(void);
SInt16 Event_QueueCount(void);
void Event_DumpQueue(void);
Boolean GetNextEvent(EventMask eventMask, EventRecord* theEvent);
Boolean EventAvail(EventMask eventMask, EventRecord* theEvent);
OSErr PostEvent(EventKind what, UInt32 message);
void FlushEvents(EventMask whichMask, EventMask stopMask);

extern EventManagerState gEventState;

#endif
