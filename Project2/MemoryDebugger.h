#pragma once

#include <vector>

#include "mallocator.h"

struct MemoryDebugger {
    MemoryDebugger();
    ~MemoryDebugger() { DetectMemoryLeaks(); }
    struct Allocation {
        Allocation() : array_allocation(false), line_number(-1) { }
        bool operator==(const Allocation& rhs) const {
            return memory_address == rhs.memory_address && return_address == rhs.return_address;
        }
        bool operator==(MemoryAddress address) const {
            return memory_address == address;
        }
        //
        bool array_allocation = false;
        MemoryAddress memory_address = nullptr;
        MemoryAddress return_address = nullptr;
        size_t allocation_size = 0;
        int line_number = -1;
        std::basic_string<char, std::char_traits<char>, Mallocator<char>> file;
        std::basic_string<char, std::char_traits<char>, Mallocator<char>> function;
    };
    struct Deallocation {
        bool deallocated;
        MemoryAddress memory_address;
        bool operator==(MemoryAddress address) const {
            return memory_address == address;
        }
    };

    typedef std::vector<Allocation, Mallocator<Allocation>> AllocationList;
    typedef std::vector<Deallocation, Mallocator<Deallocation>> DeallocationList;
    AllocationList allocation_list;
    DeallocationList deallocation_list;

	MemoryAddress Allocate(size_t, bool, MemoryAddress, const char* file = "", const char* function = "", int line = -1);
    void Deallocate(MemoryAddress, bool);
    void Deallocate(MemoryAddress, size_t, bool);

private:
    bool DetectMemoryLeaks();
    bool DoubleDeletion(MemoryAddress);
    bool InvalidDeletion(MemoryAddress);
    bool IncorrectDeletion(MemoryAddress, bool);
    bool ValidateDeletion(MemoryAddress, bool);
    void RemoveFromDeallocationList(MemoryAddress);
    void RemoveFromAllocationList(MemoryAddress);

    int num_allocations = 0;
    int num_deallocations = 0;
    size_t total_allocated_memory = 0;
};
