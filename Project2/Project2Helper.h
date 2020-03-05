#pragma once

// if we are compiling with MSVC/Windows
#if defined (_MSC_VER)

#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "dbghelp.lib")
#include <DbgHelp.h> // to load symbols
#include <intrin.h>  // for _ReturnAddress()

#pragma intrinsic(_ReturnAddress)

#define BUILTIN_RETURN_ADDR(...) _ReturnAddress()
#define BUILTIN_FILE() __FILE__
#define BUILTIN_FUNCTION() __FUNC__
#define BUILTIN_LINE() __LINE__
#define DEBUG_BREAKPOINT() __debugbreak()

#define NO_THROW noexcept
//throw specification is deprecated so this defines to nothing
#define THROWS() 
// else - we are NOT compiling with MSVC/Windows
#else

#include <sys/mman.h> // for mmap/munmap
#include <execinfo.h> // for backtrace (debug info)
#include <csignal>    // for std::raise(SIGTRAP)

// #include <x86intrin.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/resource.h>
// #include <unistd.h>

//pass the depth you want to return
#define BUILTIN_RETURN_ADDR(...) __builtin_return_address(__VA_ARGS__)
#define BUILTIN_FILE() __builtin_FILE()
#define BUILTIN_FUNCTION() __builtin_FUNCTION()
#define BUILTIN_LINE() __builtin_LINE()
#define DEBUG_BREAKPOINT() std::raise(SIGTRAP)

// if we are compiling with clang
#if defined (__clang__)

#define NO_THROW _GLIBCXX_USE_NOEXCEPT
#define THROWS() _GLIBCXX_USE_NOEXCEPT

// else if we are compiling with gnu
#elif defined (__GNUC__)

#define NO_THROW _GLIBCXX_USE_NOEXCEPT
#define THROWS() _GLIBCXX_THROW()

#endif

#endif

#define _STR(X) #X
#define STR(X) _STR(X)

#define UNUSED(expr) do { (void)(expr); } while (0)

typedef void* MemoryAddress;

#define GET_LINE_INFO() __FILE__ "(" STR(__LINE__) ")"
