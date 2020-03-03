#include <iostream>

#include "SomeUsefulIncludes.h"
#include "tests.h"


#include "MemoryDebugger.h"

MemoryDebugger memory_debugger;

void* operator new(size_t size)
{
	void* result = memory_debugger.Allocate(size, false, (MemoryAddress)BUILTIN_RETURN_ADDR(0));
	if (result == NULL) throw std::bad_alloc();
	return result;
}
void* operator new(size_t size, const std::nothrow_t&)
{
	return memory_debugger.Allocate(size, false, (MemoryAddress)BUILTIN_RETURN_ADDR(0));
}
void* operator new[](size_t size)
{
	// std::cout << "========================================== 3 =================================\n";
	void* result = memory_debugger.Allocate(size, true, (MemoryAddress)BUILTIN_RETURN_ADDR(0));
	if (result == NULL) throw std::bad_alloc();
	return result;
}
void* operator new[](size_t size, const std::nothrow_t&)
{
	return memory_debugger.Allocate(size, true, (MemoryAddress)BUILTIN_RETURN_ADDR(0));
}
void operator delete(MemoryAddress address)
{
	memory_debugger.Deallocate(address, false);
}
void operator delete(MemoryAddress address, const std::nothrow_t&)
{
	memory_debugger.Deallocate(address, false);
}
void operator delete[](MemoryAddress address)
{
	memory_debugger.Deallocate(address, true);
}
void operator delete[](MemoryAddress address, const std::nothrow_t&)
{
	memory_debugger.Deallocate(address, true);
}
void* operator new(size_t size, const char* file, const char* function, int line)
{
	void* result = memory_debugger.Allocate(size, false, (MemoryAddress)BUILTIN_RETURN_ADDR(0), file, function, line);
	if (result == NULL) throw std::bad_alloc();
	return result;
}
void* operator new(size_t size, const std::nothrow_t&, const char* file, const char* function, int line)
{
	return memory_debugger.Allocate(size, false, (MemoryAddress)BUILTIN_RETURN_ADDR(0), file, function, line);
}
void* operator new[](size_t size, const char* file, const char* function, int line)
{
	void* result = memory_debugger.Allocate(size, true, (MemoryAddress)BUILTIN_RETURN_ADDR(0), file, function, line);
	if (result == NULL) throw std::bad_alloc();
	return result;
}
void* operator new[](size_t size, const std::nothrow_t&, const char* file, const char* function, int line)
{
	return memory_debugger.Allocate(size, true, (MemoryAddress)BUILTIN_RETURN_ADDR(0), file, function, line);
}

#define new new(__FILE__, __FUNCTION__, __LINE__)

// enables/disables the nothrow test
// To enable this test, return true from the ImplementedWithNoThrowNew() function below
// You are NOT required to run this test
// However, you should implement a conformant nothrow new regardless
bool ImplementedWithNoThrowNew()
{
	return false;
}

// Pass 0-11 to choose the scenario
// IE: project2.exe 5
int main(int argc, char *argv[])
{
	int scenario = 1; // Or change this line to pick a scenario

	// Test Harness
	//======== BEGIN: DO NOT MODIFY THE FOLLOWING LINES =========//
	/**/if (argc > 1) {										   /**/
	/**/	scenario = std::atoi(argv[1]);					   /**/
	/**/}													   /**/
	/**/switch (scenario) {									   /**/
	/**/	case 11: project2_randompointer2(); break;		   /**/
	/**/	case 10: project2_randompointer1(); break;		   /**/
	/**/	case 9:  project2_newdeletevector(); break;		   /**/
	/**/	case 8:  project2_newvectordelete(); break;		   /**/
	/**/	case 7:  project2_doublevectordelete(); break;	   /**/
	/**/	case 6:  project2_doubledelete(); break;		   /**/
	/**/	case 5:  project2_deletedmemoryread(); break;	   /**/
	/**/	case 4:  project2_deletedmemorywrite(); break;	   /**/
	/**/	case 3:  project2_readoverflow(); break;		   /**/
	/**/	case 2:  project2_writeoverflow(); break;		   /**/
	/**/	case 1:  project2_leaks(); break;				   /**/
	/**/	default: project2_good(); break;				   /**/
	/**/}													   /**/
	//========  END: DO NOT MODIFY THE PREVIOUS LINES ===========//

	return 0;
}
