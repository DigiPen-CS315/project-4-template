#pragma once

#include <vector>
#include <map>
#include "mallocator.h"

class MemoryDebugger {
public:
	MemoryDebugger();
	~MemoryDebugger();

	void* Allocate(size_t size, bool isArray, void* returnAddress);
	bool Dellocate(void* address, bool isArray, size_t size = 0);

private:
	struct Allocation {
		bool operator==(const Allocation& rhs) const {
			return dataPointer == rhs.dataPointer;
		}
		bool operator==(const void* address) const {
			return dataPointer == address;
		}


		bool deleted = false;
		bool isArray = false;
		size_t dataSize = 0;
		void* dataPointer = nullptr;
		void* basePointer = nullptr;
		void* returnAddress = nullptr;
		std::basic_string<char, std::char_traits<char>, Mallocator<char>> info;
	};

	std::vector<Allocation, Mallocator<Allocation>> allocations;
};