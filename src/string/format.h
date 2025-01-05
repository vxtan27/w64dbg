/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

static const char _EXCEPTION_ACCESS_VIOLATION[16] = "access violation";
static const char _EXCEPTION_IN_PAGE_ERROR[13] = "in page error";
static const char _EXCEPTION_INVALID_HANDLE[14] = "invalid handle";
static const char _EXCEPTION_ILLEGAL_INSTRUCTION[19] = "illegal instruction";
static const char _EXCEPTION_INVALID_DISPOSITION[19] = "invalid disposition";
static const char _EXCEPTION_ARRAY_BOUNDS_EXCEEDED[21] = "array bounds exceeded";
static const char _EXCEPTION_FLT_DENORMAL_OPERAND[31] = "floating-point denormal operand";
static const char _EXCEPTION_FLT_DIVIDE_BY_ZERO[31] = "floating-point division by zero";
static const char _EXCEPTION_FLT_INEXACT_RESULT[29] = "floating-point inexact result";
static const char _EXCEPTION_FLT_INVALID_OPERATION[32] = "floating-point invalid operation";
static const char _EXCEPTION_FLT_OVERFLOW[23] = "floating-point overflow";
static const char _EXCEPTION_FLT_STACK_CHECK[26] = "floating-point stack check";
static const char _EXCEPTION_FLT_UNDERFLOW[24] = "floating-point underflow";
static const char _EXCEPTION_INT_DIVIDE_BY_ZERO[24] = "integer division by zero";
static const char _EXCEPTION_INT_OVERFLOW[16] = "integer overflow";
static const char _EXCEPTION_PRIV_INSTRUCTION[22] = "privileged instruction";
static const char _EXCEPTION_STACK_OVERFLOW[14] = "stack overflow";
static const char _EXCEPTION_POSSIBLE_DEADLOCK[27] = "possible deadlock condition";
static const char _STATUS_STACK_BUFFER_OVERRUN[9] = "fast fail";
static const char _STATUS_FATAL_USER_CALLBACK_EXCEPTION[29] = "fatal user callback exception";
static const char _STATUS_ASSERTION_FAILURE[17] = "assertion failure";
static const char _STATUS_CLR_EXCEPTION[13] = "CLR exception";
static const char _STATUS_CPP_EH_EXCEPTION[32] = "C++ exception handling exception";
static const char _EXCEPTION_GUARD_PAGE[20] = "guard page exception";
static const char _EXCEPTION_DATATYPE_MISALIGNMENT[15] = "alignment fault";
static const char _EXCEPTION_BREAKPOINT[10] = "breakpoint";
static const char _EXCEPTION_SINGLE_STEP[48] = "trace trap or other single-instruction mechanism";
static const char _STATUS_FATAL_APP_EXIT[21] = "fast application exit";
static const char _MS_VC_EXCEPTION[21] = "thread name exception";
static const char _RPC_S_UNKNOWN_IF[17] = "unknown interface";
static const char _RPC_S_SERVER_UNAVAILABLE[18] = "server unavailable";
static const char _EXCEPTION_UNKNOWN[21] = "unknown exception (0x";

static const char EXCEPTION_READING_FROM[12] = "reading from";
static const char EXCEPTION_WRITING_TO[10] = "writing to";
static const char EXCEPTION_DEP_VIOLATION[16] = "DEP violation at";
static const char EXCEPTION_LOCATION[10] = " location ";

static const char EXCEPTION_ACCESS_VIOLATION_[109] = "The thread tried to read from or write to a virtual address for which it does not have the appropriate access";
static const char EXCEPTION_IN_PAGE_ERROR_[98] = "The thread tried to access a page that was not present, and the system was unable to load the page";
static const char EXCEPTION_INVALID_HANDLE_[60] = "The thread used a handle to a kernel object that was invalid";
static const char EXCEPTION_ILLEGAL_INSTRUCTION_[50] = "The thread tried to execute an invalid instruction";
static const char EXCEPTION_INVALID_DISPOSITION_[80] = "An exception handler returned an invalid disposition to the exception dispatcher";
static const char EXCEPTION_ARRAY_BOUNDS_EXCEEDED_[118] = "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking";
static const char EXCEPTION_FLT_DENORMAL_OPERAND_[152] = "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value";
static const char EXCEPTION_FLT_DIVIDE_BY_ZERO_[85] = "The thread tried to divide a floating-point value by a floating-point divisor of zero";
static const char EXCEPTION_FLT_INEXACT_RESULT_[92] = "The result of a floating-point operation cannot be represented exactly as a decimal fraction";
static const char EXCEPTION_FLT_INVALID_OPERATION_[67] = "This exception represents any floating-point exception not included";
static const char EXCEPTION_FLT_OVERFLOW_[106] = "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type";
static const char EXCEPTION_FLT_STACK_CHECK_[79] = "The stack overflowed or underflowed as the result of a floating-point operation";
static const char EXCEPTION_FLT_UNDERFLOW_[103] = "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type";
static const char EXCEPTION_INT_DIVIDE_BY_ZERO_[73] = "The thread tried to divide an integer value by an integer divisor of zero";
static const char EXCEPTION_INT_OVERFLOW_[95] = "The result of an integer operation caused a carry out of the most significant bit of the result";
static const char EXCEPTION_PRIV_INSTRUCTION_[101] = "The thread tried to execute an instruction whose operation is not allowed in the current machine mode";
static const char EXCEPTION_STACK_OVERFLOW_[28] = "The thread used up its stack";
static const char EXCEPTION_POSSIBLE_DEADLOCK_[52] = "The wait operation on the critical section times out";
static const char STATUS_STACK_BUFFER_OVERRUN_[74] = "The system detected an overrun of a stack-based buffer in this application";
static const char STATUS_FATAL_USER_CALLBACK_EXCEPTION_[61] = "An unhandled exception was encountered during a user callback";
static const char STATUS_ASSERTION_FAILURE_[33] = "An assertion failure has occurred";
static const char STATUS_CLR_EXCEPTION_[76] = "A managed code exception was encountered within .NET Common Language Runtime";
static const char STATUS_CPP_EH_EXCEPTION_[62] = "A C++ exception has been thrown and is being handled or caught";
static const char EXCEPTION_GUARD_PAGE_[65] = "The thread accessed memory allocated with the PAGE_GUARD modifier";
static const char EXCEPTION_DATATYPE_MISALIGNMENT_[101] = "The thread tried to read or write data that is misaligned on hardware that does not provide alignment";
static const char EXCEPTION_BREAKPOINT_[28] = "A breakpoint was encountered";
static const char EXCEPTION_SINGLE_STEP_[98] = "A trace trap or other single-instruction mechanism signaled that one instruction has been executed";
static const char STATUS_WX86_BREAKPOINT_[38] = "A Win32 x86 breakpoint was encountered";
static const char STATUS_FATAL_APP_EXIT_[69] = "The application caused an unhandled runtime exception during shutdown";
static const char MS_VC_EXCEPTION_[48] = "The thread set its own name by raising exception";
static const char RPC_S_UNKNOWN_IF_[24] = "The interface is unknown";
static const char RPC_S_SERVER_UNAVAILABLE_[29] = "The RPC server is unavailable";

static const wchar_t OBJECT_MANAGER_NAMESPACE[4] = L"\\??\\";