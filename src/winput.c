/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "resrc.h" // Resource
#include "ntdll.h" // Native

#define W64DBG_KEY_MESSAGE "\nPress any key to continue ..."

#define SecToUnits(lSeconds) (lSeconds * 10000000LL)

#define IsInputValidate(InputRecord) ( \
    InputRecord.EventType == KEY_EVENT && \
    InputRecord.Event.KeyEvent.bKeyDown && \
    ( \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= 0x30 && InputRecord.Event.KeyEvent.wVirtualKeyCode <= 0x5A) || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_RETURN || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_PAUSE || \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= VK_NUMPAD0 && InputRecord.Event.KeyEvent.wVirtualKeyCode <= VK_NUMPAD9) || \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= VK_OEM_1 && InputRecord.Event.KeyEvent.wVirtualKeyCode <= VK_OEM_3) || \
        (InputRecord.Event.KeyEvent.wVirtualKeyCode >= VK_OEM_4 && InputRecord.Event.KeyEvent.wVirtualKeyCode <= VK_OEM_8) || \
        InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_OEM_102 \
    ) \
)

static __forceinline VOID WaitForInputOrTimeout(
    _In_ HANDLE hStdin,
    _In_ HANDLE hStdout,
    _In_ char StdinConsole,
    _In_ int timeout
    )
{
    NtWriteFile(hStdout, NULL, NULL, NULL, _alloca(sizeof(IO_STATUS_BLOCK)),
        W64DBG_KEY_MESSAGE, strlen(W64DBG_KEY_MESSAGE), NULL, NULL);

    if (StdinConsole)
    {
        DWORD dwRead;
        INPUT_RECORD InputRecord;

        if (timeout == -1) while (TRUE)
        {
            ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            if (IsInputValidate(InputRecord)) break;
        } else
        {
            LARGE_INTEGER DelayInterval;

            NtQuerySystemTime(&DelayInterval);
            // Convert seconds to 100-nanosecond units
            // (positive for absolute time)
            DelayInterval.QuadPart += SecToUnits(timeout);

            while (TRUE)
            {
                if (NtWaitForSingleObject(hStdin, FALSE,
                    &DelayInterval) == STATUS_TIMEOUT) break;
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
                if (IsInputValidate(InputRecord)) break;
            }
        }
    } else
    {
        // Convert seconds to 100-nanosecond units
        // (negative for relative time)
        NtWaitForSingleObject(hStdin, FALSE,
            &(LARGE_INTEGER){.QuadPart=-SecToUnits(timeout)});
    }

    // Simulate pause / timeout -1 behavior
    NtWriteFile(hStdout, NULL, NULL, NULL, _alloca(sizeof(IO_STATUS_BLOCK)),
        W64DBG_KEY_MESSAGE, 1, NULL, NULL);
}