#include <iostream>
#include <cstring>

#include "tests.h"
#include "SomeUsefulIncludes.h"

// Scenarios
// BEGIN: DO NOT TO MODIFY THIS FILE
/*****************************************************************************/

#if INTPTR_MAX == INT64_MAX
//1 TB
#define UNSERVICEABLE_VALUE 0xF000000000000000
#else
//4GB
#define UNSERVICEABLE_VALUE 0xF0000000
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)


/* Memory Leaks */
// DO NOT MODIFY
static double* project2_leaks_helper()
{
	return new double;
}

// DO NOT MODIFY
void project2_leaks()
{
	int* ints[10] = { nullptr };
	// 10 allocations - 9 deletes
	for (int i = 0; i < 10; i++)
	{
		ints[i] = new int;
	}
	for (int i = 0; i < 9; i++)
	{
		delete ints[i];
		ints[i] = nullptr;
	}

	// 10 vector allocations - 9 vector deletes
	for (int i = 0; i < 10; i++)
	{
		ints[i] = new int[i + 1];
	}
	for (int i = 0; i < 9; i++)
	{
		delete[] ints[i];
		ints[i] = nullptr;
	}

	// 1 allocation through helper function
	double* d = project2_leaks_helper();
	UNUSED(d);

	// All total - 3 leaks
}

/*****************************************************************************/
/* Overflows */

// DO NOT MODIFY
void project2_writeoverflow()
{
	int* ints = new int[10];
	// Valid Access
	for (int i = 0; i < 10; i++)
	{
		ints[i] = i;
	}

	// Write overflow
	for (int i = 1; i <= 10; i++)
	{
		ints[i] = ints[i - 1];
	}

	delete[] ints;
}

// DO NOT MODIFY
void project2_readoverflow()
{
	int* ints = new int[10];
	// Valid Access
	for (int i = 0; i < 10; i++)
	{
		ints[i] = i;
	}

	// Read overflow
	for (int i = 0; i <= 10; i++)
	{
		std::cout << "ints " << i << " is " << ints[i] << std::endl;
	}

	delete[] ints;
}

/*****************************************************************************/
/* Deleted Memory Access */

// DO NOT MODIFY
void project2_deletedmemorywrite()
{
	int* i = new int;
	*i = 42;
	delete i;

	// Write Access to Deleted Pointer
	*i = 43;
}

// DO NOT MODIFY
void project2_deletedmemoryread()
{
	int* i = new int;
	*i = 42;
	delete i;

	// Read Access to Deleted Pointer
	printf("Deleted i is: %d\n", *i);
}

/*****************************************************************************/
/* Double Delete */

// DO NOT MODIFY
void project2_doubledelete()
{
	int* i = new int;
	delete i;

	// Double Delete
	delete i;
}

// DO NOT MODIFY
void project2_doublevectordelete()
{
	int* i = new int[2];
	delete[] i;

	// Double Delete
	delete[] i;
}

/*****************************************************************************/
/* Mismatched new/delete[] and new[]/delete */

// DO NOT MODIFY
void project2_newvectordelete()
{
	int* ints = new int[2];

	// Mismatched new[]/delete
	delete ints;
}

// DO NOT MODIFY
void project2_newdeletevector()
{
	int* ints = new int;

	// Mismatched new/delete[]
	delete[] ints;
}

/*****************************************************************************/
/* Random Pointer Delete */

// DO NOT MODIFY
void project2_randompointer1()
{
	int local_int = 0;
	int* i = &local_int;

	// Cannot delete a stack variable
	delete i;
}

// DO NOT MODIFY
void project2_randompointer2()
{
	int* i = new int;
	if (i) std::cout << *i << std::endl;

	// Move the pointer backward by 1 byte - no longer i. Forward would trigger an AccessViolation.
	int* bad_i = (int*)((char*)i - 1);

	if (bad_i) std::cout << *bad_i << std::endl;

	// Cannot delete a pointer that was not returned from new
	delete bad_i;
}

/************************************d****************************************/
/* GOOD Usage - There is nothing wrong with this test */

// DO NOT MODIFY
void project2_good()
{
	// Simple Allocation Test
	int* i = new int;
	*i = 42;
	delete i;
	i = nullptr;

	// Simple Vector Allocation Test
	i = new int[2];
	i[0] = 0;
	i[1] = i[0] + 1;
	delete[] i;
	i = nullptr;

	// delete NULL test
	i = nullptr;
	delete i;

	// Large array test - ~16K
	i = new int[8000];
	std::memset(i, 0x00, 8000 * sizeof(int));
	std::memset(i, 0xFF, 8000 * sizeof(int));
	delete[] i;
	i = nullptr;

	// Very large array test - ~4MB
	i = new int[1024 * 1024];
	std::memset(i, 0x00, 1024 * 1024 * sizeof(int));
	std::memset(i, 0xFF, 1024 * 1024 * sizeof(int));
	delete[] i;
	i = nullptr;

	// Zero byte allocation must be non-NULL and Unique
	i = new int[0];
	int* i2 = new int[0];
	// If this breakpoint triggers, you will fail Scenario 12
	if (i == i2)
	{
		std::cout << "If this triggers, you will fail Scenario 12" << std::endl;
		DEBUG_BREAKPOINT();
	}
	delete[] i;
	delete[] i2;

	// bad_alloc test
	bool bad_alloc_caught = false;
	try
	{
		// a 4GB (x86) or 1TB (x64) request is simply not serviceable
		size_t s = (size_t)UNSERVICEABLE_VALUE;
		char* c = new char[s];
		UNUSED(c);
	}
	catch (std::bad_alloc)
	{
		bad_alloc_caught = true;
	}
	// If this breakpoint triggers, you will fail Scenario 12
	if (!bad_alloc_caught)
	{
		std::cout << "If this triggers, you will fail Scenario 12" << std::endl;
		DEBUG_BREAKPOINT();
	}

	// nothrow test
	// To enable this test, return true from the ImplementedWithNoThrowNew() function in main
	// You are NOT required to run this test
	// Because it uses the placement new syntax, it will break #define new
	// However, you should implement a conformant nothrow new regardless
	if (ImplementedWithNoThrowNew())
	{
		bad_alloc_caught = false;
		try
		{
			// a 4GB (x86) or 1TB (x64) request is simply not serviceable
			size_t s = (size_t)UNSERVICEABLE_VALUE;
			char* c = new (std::nothrow) char[s];
			if (c != NULL)
			{
				std::cout << "If this triggers, you will fail Scenario 12" << std::endl;
				DEBUG_BREAKPOINT();
			}
		}
		catch (std::bad_alloc)
		{
			bad_alloc_caught = true;
		}
		// If this breakpoint triggers, you will fail Scenario 12
		if (bad_alloc_caught)
		{
			std::cout << "If this triggers, you will fail Scenario 12" << std::endl;
			DEBUG_BREAKPOINT();
		}
	} // ENABLE_NOTHROW_TEST
}

// END: DO NOT MODIFY THE PREVIOUS 13 FUNCTIONS
