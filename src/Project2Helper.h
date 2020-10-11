#pragma once

// if we are compiling with MSVC/Windows
#if defined (_MSC_VER)

#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "dbghelp.lib")
#include <DbgHelp.h> // to load symbols
#include <intrin.h>  // for _ReturnAddress()

#pragma intrinsic(_ReturnAddress)

#define GET_RETURN_ADDR(...) _ReturnAddress()
#define GET_FILE_INFO() __FILE__
#define GET_FUNCTION_INFO() __FUNCTION__
#define GET_LINE_INFO() __LINE__

#define DEBUG_BREAKPOINT() __debugbreak()

#define NO_THROW noexcept
#define THROWS 


// else - we are NOT compiling with MSVC/Windows
#else

#include <sys/mman.h> // for mmap/munmap
#include <execinfo.h> // for backtrace (debug info)
#include <csignal>    // for std::raise(SIGTRAP)

// other maybe useful headers
// #include <x86intrin.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/resource.h>
// #include <unistd.h>

//pass the depth you want to return
#define GET_RETURN_ADDR() __builtin_return_address(0)
#define GET_FILE_INFO() __builtin_FILE()
#define GET_FUNCTION_INFO() __builtin_FUNCTION()
#define GET_LINE_INFO() __builtin_LINE()

#define DEBUG_BREAKPOINT() std::raise(SIGTRAP) // __builtin_trap() 

// if we are compiling with clang
#if defined (__clang__)

#define NO_THROW _GLIBCXX_USE_NOEXCEPT
#define THROWS  //_GLIBCXX_THROW() // _GLIBCXX_USE_NOEXCEPT

// else if we are compiling with gnu
#elif defined (__GNUC__)

#define NO_THROW _GLIBCXX_USE_NOEXCEPT
#define THROWS _GLIBCXX_THROW()

#endif

#endif

#define _STR(X) #X
#define STR(X) _STR(X)

#define UNUSED(expr) do { (void)(expr); } while (0)

#define GET_FILE_LINE_INFO() __FILE__ "(" STR(__LINE__) ")"

#define DEBUG_ERROR(MSG) \
	do { printf("Error: %s\n\t%s(%d)\n", MSG, __FILE__, __LINE__); DEBUG_BREAKPOINT(); } while(0) 	
