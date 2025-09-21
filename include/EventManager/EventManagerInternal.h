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

extern EventManagerState gEventState;

#endif
