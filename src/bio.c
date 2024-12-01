#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverlapped)
{
    (void) dwErr;
    (void) cbWritten;
    (void) lpOverlapped;
}

// winuser.h
/*
#define VK_CONTROL 0x11
#define VK_MENU 0x12
*/

#define IsInputValidate(InputRecord) (InputRecord.EventType == KEY_EVENT && \
    InputRecord.Event.KeyEvent.bKeyDown && \
    InputRecord.Event.KeyEvent.wVirtualKeyCode != 0x12 && \
    InputRecord.Event.KeyEvent.wVirtualKeyCode != 0x11)

#pragma warning(suppress: 5045)
static inline VOID WaitForInputOrTimeout(HANDLE hStdout, int timeout, char Console, LPOVERLAPPED lpOverlapped)
{

    if (Console)
    {
        DWORD dwRead;
        HANDLE hStdin;
        INPUT_RECORD InputRecord;

        WriteConsole(hStdout,
            "\nPress any key to continue ...", 30, NULL, NULL);
        hStdin = GetStdHandle(STD_INPUT_HANDLE);

        if (timeout == -1) while (TRUE)
        {
            ReadConsoleInputA(hStdin, &InputRecord, 1, &dwRead);
            if (IsInputValidate(InputRecord)) break;
        } else
        {
            DWORD ctime, wtime = GetTickCount() + (timeout << 10);
            while (TRUE)
            {
                ctime = GetTickCount();
                if (wtime <= ctime || WaitForSingleObjectEx(hStdin,
                    wtime - ctime, FALSE) == WAIT_TIMEOUT) break;
                ReadConsoleInputA(hStdin, &InputRecord, 1, &dwRead);
                if (IsInputValidate(InputRecord)) break;
            }
        }

        WriteConsole(hStdout,
            "\nPress any key to continue ...", 1, NULL, NULL);
    } else
    {
        WriteFileEx(hStdout, "\nPress any key to continue ...",
            30, lpOverlapped, CompletedWriteRoutine);
        SleepEx(timeout, FALSE);
    }
}