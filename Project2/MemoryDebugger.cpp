
#include <fstream>
#include <cstdarg>
#include <iostream>
#include <algorithm>

#if defined (_MSC_VER)

#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "dbghelp.lib")
#include <DbgHelp.h>


#else

#include <x86intrin.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <csignal>

#endif

#include "SomeUsefulIncludes.h"
#include "MemoryDebugger.h"

// static void LogError(const char* error_message, unsigned long error_code)
// {
//     std::fstream file_stream;
//     file_stream.open ("log.txt", std::fstream::app);
//     char buffer[255];
//     int size = sprintf(buffer, "Error: %s. Code: %016lX.\n\n", error_message, error_code);
//     file_stream.write(buffer, size);
//     file_stream.close();
// }
//
// 
MemoryDebugger::MemoryDebugger() : num_allocations(0), num_deallocations(0), total_allocated_memory(0) {
#if defined (_MSC_VER)
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitialize(GetCurrentProcess(), NULL, true)) {
		std::cout << "Error: SymInitialize() failed" << std::endl;
		//     Log("Error: SymInitialize() failed");
	}
#endif
}

void Log(const char* format, ...)
{
    std::ofstream output;
    char buffer[255];
    va_list args;
    va_start(args, format);
    sprintf(buffer, format, args);
    va_end(args);
    output.open("debug_log.log", std::ofstream::app);
    output << buffer;
    output.close();
}

static void GetAllocationSize(size_t* allocation_size, size_t size)
{
    *allocation_size = size / 4096;
    if (size % 4096) ++*allocation_size;
    ++*allocation_size;
    *allocation_size *= 4096;
}

static MemoryAddress AllocatePageAligned(size_t* allocation_size, size_t size)
{
    GetAllocationSize(allocation_size, size);
#if defined (_MSC_VER)
	MemoryAddress address = VirtualAlloc(0, *allocation_size * 2, MEM_RESERVE, PAGE_NOACCESS);
	address = VirtualAlloc(address, *allocation_size, MEM_COMMIT, PAGE_READWRITE);
	if (address == NULL)
		return address;
#else
    MemoryAddress address = mmap(NULL, *allocation_size * 2, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (address == NULL)
        return address;
    int e = mprotect(address, *allocation_size, PROT_READ | PROT_WRITE);
    if (e < 0)
        return NULL;
#endif
    return static_cast<unsigned char*>(address) + *allocation_size - size;
}

MemoryAddress MemoryDebugger::Allocate(size_t size, bool array_allocation, MemoryAddress return_address) {
	++num_allocations;
	total_allocated_memory += size;
	Allocation a;
	a.return_address = return_address;
	a.line_number = reinterpret_cast<long>(return_address);
	// a.file = file;
	// a.function = function;

	a.memory_address = ::AllocatePageAligned(&a.allocation_size, size);
	if (a.memory_address == NULL)
		return NULL;
	a.array_allocation = array_allocation;
	allocation_list.push_back(a);
	for (unsigned i = 0; i < deallocation_list.size(); ++i)
	{
		if (a.memory_address == deallocation_list[i].memory_address)
		{
			deallocation_list[i].deallocated = false;
			return a.memory_address;
		}
	}
	
	Deallocation d;
	d.deallocated = false;
	d.memory_address = a.memory_address;
	deallocation_list.push_back(d);
	return a.memory_address;
}

MemoryAddress MemoryDebugger::Allocate(size_t size, bool array_allocation, MemoryAddress return_address, const char* file, const char* function, int line) {
	++num_allocations;
	total_allocated_memory += size;
	Allocation a;
	a.return_address = return_address;
	a.line_number = line;
	a.file = file;
	a.function = function;

	a.memory_address = ::AllocatePageAligned(&a.allocation_size, size);
	if (a.memory_address == NULL)
		return NULL;
	a.array_allocation = array_allocation;
	allocation_list.push_back(a);
	for (unsigned i = 0; i < deallocation_list.size(); ++i)
	{
		if (a.memory_address == deallocation_list[i].memory_address)
		{
			deallocation_list[i].deallocated = false;
			return a.memory_address;
		}
	}
	
	Deallocation d;
	d.deallocated = false;
	d.memory_address = a.memory_address;
	deallocation_list.push_back(d);
	return a.memory_address;
}

// static IMAGEHLP_LINE64 GetSymbols(unsigned long long address) 
//{
//     SymSetOptions(SYMOPT_LOAD_LINES);
//     IMAGEHLP_LINE64 line;
//     line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
//     DWORD displacement;
//     if (!SymGetLineFromAddr64(getpid(), address, &displacement, &line)) {
//         unsigned long error = GetLastError();
//         std::cout << "SymGetLineFromAddr64() failed.  Error: " << error << std::endl;
//         //Log("SymGetLineFromAddr64() failed.  Error: %016llX", error);
//     }
//     return line;
// }

bool MemoryDebugger::DetectMemoryLeaks()
{
    if (num_allocations <= num_deallocations) return false;
    std::ofstream leaks("leaks.log");
    leaks << "Memory leaks detected. \n";
    for (unsigned i = 0; i < allocation_list.size(); ++i)
	{
        // auto symbols = GetSymbols((unsigned long long)allocation_list[i].return_address);
        leaks << "Address: " << allocation_list[i].memory_address 
              << ", File: " << allocation_list[i].file 
              << ", Line: " << allocation_list[i].line_number
              << "\n";
    }
    leaks.close();
    return true;
}
bool MemoryDebugger::DoubleDeletion(MemoryAddress address)
{
	for (unsigned i = 0; i < deallocation_list.size(); ++i)
	{
		if (deallocation_list[i].memory_address == address && deallocation_list[i].deallocated)
		{
			std::cout << "Double Deletion of address: 0x" << address << std::endl;
			DEBUG_BREAKPOINT();
			return true;
		}
	}
    return false;
}
bool MemoryDebugger::InvalidDeletion(MemoryAddress address)
{
	for (unsigned i = 0; i < allocation_list.size(); ++i)
	{
		if (allocation_list[i].memory_address == address)
		{
			return false;
		}
	}
    std::cout << "Invalid Deletion of address: 0x" << address << std::endl;
    DEBUG_BREAKPOINT();
    return true;
}
bool MemoryDebugger::IncorrectDeletion(MemoryAddress address, bool array_deletion)
{
	for (unsigned i = 0; i < allocation_list.size(); ++i)
	{
		if (allocation_list[i].memory_address == address)
		{
			if (allocation_list[i].array_allocation == array_deletion)
			{
				return false;
			}
			else {
				std::cout << "Incorrect delete() on address: 0x" << address << std::endl;
				DEBUG_BREAKPOINT();
				return true;
			}
		}
	}
    return false;
}

bool MemoryDebugger::ValidateDeletion(MemoryAddress address, bool array_deletion)
{
    if (address == NULL)
	{
        std::cout << "Deallocation address is null, no delete performed" << std::endl;
        return false;
    }
    bool double_delete = !DoubleDeletion(address),
         invalid_deletion = !InvalidDeletion(address),
         incorrect_deletion = !IncorrectDeletion(address, array_deletion),
         result = double_delete && invalid_deletion && incorrect_deletion;
    return result;
}

void MemoryDebugger::RemoveFromAllocationList(MemoryAddress address)
{
	AllocationList::iterator i = std::find(allocation_list.begin(), allocation_list.end(), (MemoryAddress)address);
	allocation_list.erase(i);
}
void MemoryDebugger::RemoveFromDeallocationList(MemoryAddress address)
{
	DeallocationList::iterator i = std::find(deallocation_list.begin(), deallocation_list.end(), (MemoryAddress)address);
	i->deallocated = true;
}

static void Deallocate_DecommitPageAligned(MemoryAddress address, size_t size_to_deallocate)
{
#if defined (_MSC_VER)
	VirtualFree(address, size_to_deallocate, MEM_DECOMMIT);
#else
    munmap(address, size_to_deallocate);
#endif
}

void MemoryDebugger::Deallocate(MemoryAddress address, bool array_deletion)
{
    std::cout << "Attempting deallocation of address: 0x" << address << std::endl;
    if (ValidateDeletion(address, array_deletion) == false)
        return;
    ++num_deallocations;
    unsigned i = 0;
    for (; i < allocation_list.size(); ++i)
	{
        if (allocation_list[i].memory_address == address)
		{
            RemoveFromDeallocationList(allocation_list[i].memory_address);
            ::Deallocate_DecommitPageAligned(address, allocation_list[i].allocation_size);
            RemoveFromAllocationList(allocation_list[i].memory_address);
            std::cout << "Deallocation succeeded of address: 0x" << address << std::endl;
            return;
        }
    }
	std::cout << "No deallocation occurred of address" << address << std::endl;
	DEBUG_BREAKPOINT();
    //for (; i < allocation_list.size(); ++i)
    //    if (allocation_list[i].memory_address = x)
    //        allocation_list[i].deallocated = true;
}