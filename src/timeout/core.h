#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma once

typedef struct
{
    HANDLE hStdin;
    long timeout;
} THREAD_PARAMETER;

#define SecToUnits(lSeconds) ((lSeconds) * 10000000LL)

#define IsInputInvalidate(InputRecord) ( \
    InputRecord.EventType != KEY_EVENT || \
    !InputRecord.Event.KeyEvent.bKeyDown || \
    ( \
        InputRecord.Event.KeyEvent.wVirtualKeyCode < 0x30 && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_RETURN && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_ESCAPE && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_PAUSE) || \
    ( \
        InputRecord.Event.KeyEvent.wVirtualKeyCode > 0x5A && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode < VK_SLEEP) || \
    ( \
        InputRecord.Event.KeyEvent.wVirtualKeyCode > VK_DIVIDE && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode < VK_OEM_1) || \
    InputRecord.Event.KeyEvent.wVirtualKeyCode > VK_OEM_102 \
)

static const char InfiniteMessage[30] = "\nPress any key to continue ...";
static const char FiniteMessage[42] = " seconds, press a key to continue ...\x1b[37D";