#pragma once
#include <vector>

#include "mallocator.h"
#include <condition_variable>

typedef void*  MemoryAddress;


struct MemoryDebugger {
    MemoryDebugger();
    ~MemoryDebugger() { DetectMemoryLeaks();}
    struct Allocation {
        Allocation() : array_allocation(false), line_number(-1), file(""), function("")/*, deallocated(false)*/ { }
        bool operator==(const Allocation& rhs) const {
            return memory_address == rhs.memory_address && return_address == rhs.return_address;
        }
        bool operator==(MemoryAddress address) const {
            return memory_address == address;
        }
        //
        bool array_allocation;
        //bool deallocated;
        MemoryAddress memory_address;
        MemoryAddress return_address;
        size_t allocation_size;
        int line_number;
		const char* file;
		const char* function;
    };
    struct Deallocation {
        bool deallocated;
        MemoryAddress memory_address;
        bool operator==(MemoryAddress address) const {
            return memory_address == address;
        }
    };
    //
    typedef std::vector<Allocation, Mallocator<Allocation*> > AllocationList;
    typedef std::vector<Deallocation, Mallocator<Deallocation*> > DeallocationList;
    AllocationList allocation_list;
    DeallocationList deallocation_list;
    //
	MemoryAddress Allocate(size_t, bool, MemoryAddress);
	MemoryAddress Allocate(size_t, bool, MemoryAddress, const char * file, const char * function, int line);
    void Deallocate(MemoryAddress, bool);
    //
private:
    bool DetectMemoryLeaks();
    bool DoubleDeletion(MemoryAddress);
    bool InvalidDeletion(MemoryAddress);
    bool IncorrectDeletion(MemoryAddress, bool);
    bool ValidateDeletion(MemoryAddress, bool);
    void RemoveFromDeallocationList(MemoryAddress);
    void RemoveFromAllocationList(MemoryAddress);
    //
    int num_allocations,
        num_deallocations;
    size_t total_allocated_memory;
};
