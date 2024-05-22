#pragma once
#ifndef __DEBUG_OUTPUT__
#define __DEBUG_OUTPUT__

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <yvals.h>

#define DEBUG_ERROR 1
#define DEBUG_WARNING 2
#define DEBUG_INFO 3
#define DEBUG_VERBOSE 4

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_INFO
#endif


VOID _DBGPRINT_EXE(LPCWSTR lpcwszFunction, INT nLineNumber, INT nDebugLevel, LPCWSTR lpcwszDebugFormatString, ...);
VOID _DBGPRINT_DLL(LPCWSTR lpcwszFunction, INT nLineNumber, INT nDebugLevel, LPCWSTR lpcwszDebugFormatString, ...);


#if defined (_DEBUG) || defined (_VERBOSE)
#ifndef _CONSOLE
#define DBGPRINT(nDebugLevel, lpcwszDebugFormatString, ...) \
if (nDebugLevel<=DEBUG_LEVEL){ \
_DBGPRINT_DLL(__FUNCTIONW__, __LINE__, nDebugLevel, lpcwszDebugFormatString, __VA_ARGS__); \
}
#else
#define DBGPRINT(nDebugLevel, lpcwszDebugFormatString, ...) \
if (nDebugLevel<=DEBUG_LEVEL){ \
_DBGPRINT_EXE(__FUNCTIONW__, __LINE__, nDebugLevel, lpcwszDebugFormatString, __VA_ARGS__); \
}
#endif
#else // else not #ifdef _DEBUG
#define DBGPRINT( lpcwszDebugFormatString, ... ) 
#endif // end ifdef _DEBUG

#endif // end ifndef __DEBUG_OUTPUT__
