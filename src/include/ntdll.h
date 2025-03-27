// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include <phnt_windows.h>
#include <phnt.h>

#undef NtCurrentPeb

FORCEINLINE
PPEB
NtCurrentPeb(
    VOID
) {
#if defined(_M_AMD64)
    return (PPEB) __readgsqword(FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_IX86)
    return (PPEB) __readfsdword(FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_ARM)
    return (PPEB)(((PTEB) (ULONG_PTR) _MoveFromCoprocessor(CP15_TPIDRURW))->ProcessEnvironmentBlock);
#elif defined(_M_ARM64)
    return (PPEB)(((PTEB) __getReg(18))->ProcessEnvironmentBlock);
#elif defined(_M_IA64)
    return *(PPEB*)((size_t) _rdteb() + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_ALPHA)
    return *(PPEB*)((size_t) _rdteb() + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_MIPS)
    return *(PPEB*)((*(size_t*) (0x7ffff030)) + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_PPC)
    return *(PPEB*)(__gregister_get(13) + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#else
#error "Unsupported architecture"
#endif
}

#define NtQuerySystemTime(pSystemTime) \
    *pSystemTime = *(PLARGE_INTEGER) &USER_SHARED_DATA->SystemTime

#define RtlStandardInput() (NtCurrentPeb()->ProcessParameters->StandardInput)
#define RtlStandardOutput() (NtCurrentPeb()->ProcessParameters->StandardOutput)
#define RtlStandardError() (NtCurrentPeb()->ProcessParameters->StandardError)

#define RtlCurrentDirectory() (&NtCurrentPeb()->ProcessParameters->CurrentDirectory)
#define RtlDosPath() (&RtlCurrentDirectory()->DosPath)
#define RtlCommandLine() (&NtCurrentPeb()->ProcessParameters->CommandLine)
#define RtlEnvironment() (NtCurrentPeb()->ProcessParameters->Environment)