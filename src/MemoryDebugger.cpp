#include <iostream>
#include <fstream>
#include <new> 
#include <algorithm>
#include <vector>
#include <string>

#include "Project2Helper.h"
#include "MemoryDebugger.h"
#include "memory.h"

static const size_t PAGE_SIZE = 4096;

MemoryDebugger& GetMemDebugger()
{
	static MemoryDebugger debugger;
	return debugger;
}


static void SymbolsInit() {
#if defined (_MSC_VER)
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitialize(GetCurrentProcess(), nullptr, true)) {
		std::cout << "Error: SymInitialize() failed" << std::endl;
	}
#endif
}

#if defined (_MSC_VER)
static IMAGEHLP_LINE64 GetSymbols(DWORD64 address) {
	SymSetOptions(SYMOPT_LOAD_LINES);
	IMAGEHLP_LINE64 line;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD displacement;
	if (!SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &line)) {
		unsigned long error = GetLastError();
		std::cout << "SymGetLineFromAddr64() for address " << address << " failed. "
			<< "Error: " << error << std::endl;
	}
	return line;
}
#endif

std::basic_string<char, std::char_traits<char>, Mallocator<char>> getTraceInfo(size_t calls) {
#if !defined (_MSC_VER) // gcc/clang (linux)
	void* array[10];
	size_t size;
	char** strings;

	size = backtrace(array, 10);  // trace last 10 calls
	strings = backtrace_symbols(array, size);

	if (calls >= size)
		calls = size - 1;

	std::basic_string<char, std::char_traits<char>, Mallocator<char>> result = strings[calls];

	free(strings);

	return result;

#else // Windows

	UNUSED(calls);
	return "";
#endif
}


MemoryDebugger::MemoryDebugger() {
	SymbolsInit();
}

MemoryDebugger::~MemoryDebugger() {
	// loop through all allocations in our set
	// check "if (allocation.deallocated == false)"
	//    if false, write that out to a file 
	//      BUT, we want to resolve the function name

	std::ofstream outfile("leaks.txt");

	for (auto& allocation : allocations) {
		if (!allocation.deleted) {
#if defined (_MSC_VER)
			auto symbol = GetSymbols((DWORD64)allocation.returnAddress);
			outfile << "Memory leak detected at " << symbol.FileName << "(" << symbol.LineNumber << ")" << " of size " << allocation.dataSize << std::endl;
#else
			outfile << "Memory leak detected. Info: " << allocation.info << ". Of size " << allocation.dataSize << std::endl;
#endif
		}

		_release(allocation.basePointer, allocation.dataSize);
	}
}

void* MemoryDebugger::Allocate(size_t size, bool isArray, void* returAddress) {
	size_t numberOfPages = (size / PAGE_SIZE) + 1;
	if (size % PAGE_SIZE || size == 0) {
		++numberOfPages;
	}

	void* p = _allocate(numberOfPages * PAGE_SIZE);
	if (p == nullptr) {
		return nullptr;
	}

	p = _protect(p, (numberOfPages - 1) * PAGE_SIZE);
	if (p == nullptr) {
		return nullptr;
	}

	Allocation allocation;
	allocation.basePointer = p;
	allocation.dataSize = size;
	allocation.isArray = isArray;
	allocation.returnAddress = returAddress;
	allocation.info = getTraceInfo(2); // back 2 calls
	allocation.dataPointer = (unsigned char*)p + ((numberOfPages - 1) * PAGE_SIZE - size);

	GetMemDebugger().allocations.push_back(allocation);

	return allocation.dataPointer;
}

bool MemoryDebugger::Dellocate(void* address, bool isArray, size_t size /* = 0 */) {
	UNUSED(size);
	if (address == nullptr) {
		return true;
	}

	// loop on the vector of allocations   
	// or look up in the map using map.find

	std::vector<Allocation, Mallocator<Allocation>>::iterator allocation = std::find(allocations.begin(), allocations.end(), address);
	if (allocation == GetMemDebugger().allocations.end()) {
		// were deleting an address doesn't exist in our set (wasn't allocated with new???)
		DEBUG_ERROR("Deleting an address not allocated by us");
		return false;
	}
	if (allocation->deleted) {
		// deleting a pointer that has already been deleted and not set to null;
		DEBUG_BREAKPOINT();
	}
	if (allocation->isArray != isArray) {
		// scalar/vector mismatch
		DEBUG_BREAKPOINT();
	}

	allocation->deleted = true;

	return _deallocate(address, size);
}

#if 1

void* operator new(size_t size) THROWS {
	void* ptr = GetMemDebugger().Allocate(size, false, GET_RETURN_ADDR());
	if (ptr == nullptr) {
		throw std::bad_alloc();
	}
	return ptr;
}

void* operator new[](size_t size) THROWS{
	void* ptr = GetMemDebugger().Allocate(size, true, GET_RETURN_ADDR());
	if (ptr == nullptr) {
		throw std::bad_alloc();
	}
	return ptr;
}

void* operator new(size_t size, const std::nothrow_t&) NO_THROW {
	return GetMemDebugger().Allocate(size, false, GET_RETURN_ADDR());
}

void* operator new[](size_t size, const std::nothrow_t&) NO_THROW {
	return GetMemDebugger().Allocate(size, true, GET_RETURN_ADDR());
}


//void operator delete(void* address) THROWS {
//	GetMemDebugger().Dellocate(address, false);
//}
//
//void operator delete[](void* address) THROWS {
//	GetMemDebugger().Dellocate(address, true);
//}

//void operator delete(void* address, size_t size) THROWS {
//	GetMemDebugger().Dellocate(address, false, size);
//}
//
//void operator delete[](void* address, size_t size) THROWS {
//	GetMemDebugger().Dellocate(address, true, size);
//}
//
//void operator delete(void* address, const std::nothrow_t&) THROWS {
//	GetMemDebugger().Dellocate(address, false);
//}
//
//void operator delete[](void* address, const std::nothrow_t&) THROWS {
//	GetMemDebugger().Dellocate(address, true);
//}



void operator delete(void* address) NO_THROW {
	GetMemDebugger().Dellocate(address, false);
}

void operator delete[](void* address) NO_THROW {
	GetMemDebugger().Dellocate(address, true);
}

void operator delete(void* address, size_t size) NO_THROW {
	GetMemDebugger().Dellocate(address, false, size);
}

void operator delete[](void* address, size_t size) NO_THROW{
	GetMemDebugger().Dellocate(address, true, size);
}

void operator delete(void* address, const std::nothrow_t&) NO_THROW {
	GetMemDebugger().Dellocate(address, false);
}

void operator delete[](void* address, const std::nothrow_t&) NO_THROW{
	GetMemDebugger().Dellocate(address, true);
}

#endif
