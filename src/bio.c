#include "ntdll.h"

// winuser.h
/*
#define VK_CONTROL 0x11
#define VK_MENU 0x12
*/

#define IsInputValidate(InputRecord) (InputRecord.EventType == KEY_EVENT && \
    InputRecord.Event.KeyEvent.bKeyDown && \
    InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_MENU && \
    InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_CONTROL)

#define SecToUnits(lSeconds) (lSeconds * 10000000)

#pragma warning(suppress: 5045)
static inline VOID WaitForInputOrTimeout(HANDLE hStdout, int timeout)
{

    HANDLE hStdin;
    IO_STATUS_BLOCK IoStatusBlock;

    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
        "\nPress any key to continue ...", 30, NULL, NULL);
    hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if (GetFileType(hStdin) == FILE_TYPE_CHAR)
    {
        DWORD dwRead;
        INPUT_RECORD InputRecord;

        if (timeout == -1) while (TRUE)
        {
            ReadConsoleInputA(hStdin, &InputRecord, 1, &dwRead);
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
                ReadConsoleInputA(hStdin, &InputRecord, 1, &dwRead);
                if (IsInputValidate(InputRecord)) break;
            }
        }
    } else
    {
        LARGE_INTEGER DelayInterval;
        // Convert seconds to 100-nanosecond units
        // (negative for relative time)
        DelayInterval.QuadPart = -SecToUnits(timeout);
        NtWaitForSingleObject(hStdin, FALSE, &DelayInterval);
    }

    // Simulate pause / timeout -1 behavior
    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
        "\nPress any key to continue ...", 1, NULL, NULL);
}