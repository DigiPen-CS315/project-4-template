#pragma once

#if defined (_MSC_VER)

// #include <windows.h>
// #pragma comment(lib, "dbghelp.lib")
// #include <DbgHelp.h>
#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

#define BUILTIN_RETURN_ADDR(...) _ReturnAddress()
#define BUILTIN_FILE() __FILE__
#define BUILTIN_FUNCTION() __FUNC__
#define BUILTIN_LINE() __LINE__
#define DEBUG_BREAKPOINT() __debugbreak()

#else

// #include <sys/types.h>
// #include <sys/mman.h>
// #include <sys/types.h>
// #include <unistd.h>
// #include <execinfo.h>
// #include <cxxabi.h>
#include <csignal>

#define BUILTIN_RETURN_ADDR(...) __builtin_return_address(__VA_ARGS__)
#define BUILTIN_FILE() __builtin_FILE()
#define BUILTIN_FUNCTION() __builtin_FUNCTION()
#define BUILTIN_LINE() __builtin_LINE()

#if defined (__clang__)

#define DEBUG_BREAKPOINT() std::raise(SIGINT);

#elif defined (__GNUC__)

#define DEBUG_BREAKPOINT() __builtin_trap()

#endif

#endif
