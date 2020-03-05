#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <new> 
// #include <cstdarg>
// #include <cstdio>
// #include <cstdlib>
// #include <csignal>
// #include <cassert>
// #include <condition_variable>

#if defined (_MSC_VER)

// #define NOMINMAX
// #include <windows.h>
// #pragma comment(lib, "dbghelp.lib")
// #include <DbgHelp.h>

#else

// #include <sys/mman.h>
// #include <execinfo.h>
// #include <x86intrin.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/resource.h>
// #include <unistd.h>

#endif

#include "Project2Helper.h"
#include "MemoryDebugger.h"
#include "Memory.h"

#if defined (_MSC_VER)
static IMAGEHLP_LINE64 GetSymbols(unsigned long long address)
{
    SymSetOptions(SYMOPT_LOAD_LINES);
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement;
    if (!SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &line)) {
        unsigned long error = GetLastError();
        std::cout << "SymGetLineFromAddr64() failed.  Error: " << error << std::endl;
    }
    return line;
}

#else

std::basic_string<char, std::char_traits<char>, Mallocator<char>> get_trace(size_t j) {
    void* array[10];
    size_t size;
    char** strings;

    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);

	if (j >= size)
        j = size - 1;
	
    std::basic_string<char, std::char_traits<char>, Mallocator<char>> result = strings[j];

    free(strings);

    return result;
}

#endif

MemoryDebugger::MemoryDebugger() : num_allocations(0), num_deallocations(0), total_allocated_memory(0) {
#if defined (_MSC_VER)
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitialize(GetCurrentProcess(), nullptr, true)) {
		std::cout << "Error: SymInitialize() failed" << std::endl;
	}
#endif
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
	auto address = _allocate(allocation_size);
	address = _protect(address, allocation_size);
	if (address == nullptr)
		return nullptr;

    return static_cast<unsigned char*>(address) + *allocation_size - size;
}

MemoryAddress MemoryDebugger::Allocate(size_t size, bool array_allocation, MemoryAddress return_address, const char* file, const char* function, int line) {
	++num_allocations;
	total_allocated_memory += size;
	Allocation a;
	a.return_address = return_address;
	a.line_number = line;
	a.file = file;
	a.function = function;

#if defined (_MSC_VER)

#else
	if(a.file.empty()) {
        a.file = get_trace(2);
	}

#endif

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

bool MemoryDebugger::DetectMemoryLeaks()
{
    if (num_allocations <= num_deallocations) return false;
    std::ofstream leaks("leaks.log");
    leaks << allocation_list.size() << " memory leaks detected. \n";
    for (unsigned i = 0; i < allocation_list.size(); ++i)
	{
    	
#if defined (_MSC_VER)
    	
        auto symbols = GetSymbols((unsigned long long)allocation_list[i].return_address);
		if (allocation_list[i].file.empty())
			allocation_list[i].file = symbols.FileName;
		if (allocation_list[i].line_number == -1)
			allocation_list[i].line_number = symbols.LineNumber;
		if (allocation_list[i].allocation_size == 0)
			allocation_list[i].allocation_size = symbols.SizeOfStruct;

#else    	

#endif

        leaks << "Allocation Address: " << allocation_list[i].memory_address
              << ", Return Address: " << allocation_list[i].return_address
              << ", File: "  << allocation_list[i].file
			  << ", Line: "  << allocation_list[i].line_number
			  << ", Size: "  << allocation_list[i].allocation_size
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
    if (address == nullptr)
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
	_deallocate(address, size_to_deallocate);
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
}

void MemoryDebugger::Deallocate(MemoryAddress address, size_t size, bool array_deletion)
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
			// assert(size == allocation_list[i].allocation_size);
        	UNUSED(size);
            RemoveFromAllocationList(allocation_list[i].memory_address);
            std::cout << "Deallocation succeeded of address: 0x" << address << std::endl;
            return;
        }
    }
	std::cout << "No deallocation occurred of address" << address << std::endl;
	DEBUG_BREAKPOINT();
}


MemoryDebugger memory_debugger;


void* operator new(size_t size) {
	void* result = memory_debugger.Allocate(size, false, static_cast<MemoryAddress>(BUILTIN_RETURN_ADDR(0)));
	if (result == nullptr) throw std::bad_alloc();
	return result;
}
void* operator new(size_t size, const std::nothrow_t&) NO_THROW {
	return memory_debugger.Allocate(size, false, static_cast<MemoryAddress>(BUILTIN_RETURN_ADDR(0)));
}
void* operator new[](size_t size) {
	void* result = memory_debugger.Allocate(size, true, static_cast<MemoryAddress>(BUILTIN_RETURN_ADDR(0)));
	if (result == nullptr) throw std::bad_alloc();
	return result;
}
void* operator new[](size_t size, const std::nothrow_t&) NO_THROW {
	return memory_debugger.Allocate(size, true, static_cast<MemoryAddress>(BUILTIN_RETURN_ADDR(0)));
}
void operator delete(MemoryAddress address) THROWS() {
	memory_debugger.Deallocate(address, false);
}
void operator delete(MemoryAddress address, size_t size) THROWS() {
	memory_debugger.Deallocate(address, size, false);
}
void operator delete(MemoryAddress address, const std::nothrow_t&) NO_THROW {
	memory_debugger.Deallocate(address, false);
}
void operator delete[](MemoryAddress address) THROWS() {
	memory_debugger.Deallocate(address, true);
}
void operator delete[](MemoryAddress address, size_t size) THROWS() {
	memory_debugger.Deallocate(address, size, false);
}
void operator delete[](MemoryAddress address, const std::nothrow_t&) NO_THROW {
	memory_debugger.Deallocate(address, true);
}
