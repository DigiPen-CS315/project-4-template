#pragma once

#include <stdlib.h>

void* _allocate(size_t allocation_size);

void* _protect(void* address, size_t allocation_size);

bool _deallocate(void* address, size_t size_to_deallocate);

bool _release(void* address, size_t size_to_deallocate);
