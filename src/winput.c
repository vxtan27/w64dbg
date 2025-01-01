/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "resrc.h" // Resource
#include "ntdll.h" // Native
#include <malloc.h>

#define SecToUnits(lSeconds) ((lSeconds) * 10000000LL)

#define IsInputValidate(InputRecord) ( \
    InputRecord.EventType == KEY_EVENT && \
    InputRecord.Event.KeyEvent.bKeyDown && \
    ( \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= 0x30 && \
            InputRecord.Event.KeyEvent.wVirtualKeyCode <= 0x5A) || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_RETURN || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_PAUSE || \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= VK_NUMPAD0 && \
            InputRecord.Event.KeyEvent.wVirtualKeyCode <= VK_DIVIDE) || \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= VK_OEM_1 && \
            InputRecord.Event.KeyEvent.wVirtualKeyCode <= VK_OEM_3) || \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= VK_OEM_4 && \
            InputRecord.Event.KeyEvent.wVirtualKeyCode <= VK_OEM_8) || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_OEM_102 \
    ) \
)

#define IsInputNotValidate(InputRecord) ( \
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

static const char Message[30] = "\nPress any key to continue ...";

static
__forceinline
VOID WaitForInputOrTimeout(
    _In_ HANDLE hStdin,
    _In_ HANDLE hStdout,
    _In_ char StdinConsole,
    _In_ int timeout
    )
{
    IO_STATUS_BLOCK IoStatusBlock;

    // Write the message in one call to minimize I/O overhead
    NtWriteFile(hStdout, NULL, NULL, NULL,
        &IoStatusBlock, Message, sizeof(Message), NULL, NULL);

    if (StdinConsole)
    {
        DWORD dwRead;
        INPUT_RECORD InputRecord;

        if (timeout == -1)
        {
            do
            { // Infinite loop waiting for valid input
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputNotValidate(InputRecord));
        } else
        {
            LARGE_INTEGER DelayInterval;
            NtQuerySystemTime(&DelayInterval);
            // Positive value for Absolute timeout
            DelayInterval.QuadPart += SecToUnits(timeout);

            do
            { // Wait for either timeout or input
                if (NtWaitForSingleObject(hStdin, FALSE,
                    &DelayInterval) == STATUS_TIMEOUT) break;
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputNotValidate(InputRecord));
        }
    } else NtWaitForSingleObject(hStdin, FALSE, // Relative time
            &(LARGE_INTEGER){.QuadPart=-SecToUnits(timeout)});

    // Clear the message display by writing a single character
    NtWriteFile(hStdout, NULL, NULL, NULL,
        &IoStatusBlock, Message, 1, NULL, NULL);
}