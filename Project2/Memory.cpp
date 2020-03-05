#include <new>

#include "Project2Helper.h"
#include "Memory.h"

MemoryAddress _allocate(const size_t* allocation_size)
{
#if defined (_MSC_VER)
	return VirtualAlloc(0, *allocation_size * 2, MEM_RESERVE, PAGE_NOACCESS);
#else
	return mmap(nullptr, *allocation_size * 2, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
#endif
}

MemoryAddress _protect(MemoryAddress address, const size_t* allocation_size)
{
	if (address == nullptr)
		return nullptr;

#if defined (_MSC_VER)
	address = VirtualAlloc(address, *allocation_size, MEM_COMMIT, PAGE_READWRITE);
#else
	int e = mprotect(address, *allocation_size, PROT_READ | PROT_WRITE);
	if (e < 0)
		return nullptr;
#endif

	return address;
}

bool _deallocate(MemoryAddress address, size_t size_to_deallocate)
{
#if defined (_MSC_VER)
	return VirtualFree(address, size_to_deallocate, MEM_DECOMMIT);
#else
	int e = munmap(address, size_to_deallocate);
	return e == 0;
#endif
}
