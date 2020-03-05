#pragma once

// this serves as the interface to our abstraction layer

MemoryAddress _allocate(const size_t* allocation_size);
MemoryAddress _protect(MemoryAddress address, const size_t* allocation_size);
bool _deallocate(MemoryAddress address, size_t size_to_deallocate);
