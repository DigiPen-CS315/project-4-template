# Project 2 - Memory Debugger

## Compilers  
- Cover which compilers your application targets
    - [ ] gcc [version]  
    - [ ] clang [version]  
    - [ ] msvc/Visual Studio [version]  
    

## Integration  
- Cover how to integrate the memory debugger to a new project (replace this text)

## Requirements  
- What if any code/project changes are required to add it to an existing project (replace this text)

## Output  
- What is logged in the event of an error or leak (replace this text)
- The type output file, naming convention, etc. (replace this text)

## Additional Information
### Areas of difficulty:

### How many hours the project took you:

---
# CS315 Project 2 Memory Debugger Secondary Handout
By Roland Shum

## Document Goal:
The purpose of this document is to help students gain a foothold on how to start this project. This document will show you how to get started
using the non-macro method of **overloading** new()/delete().


## Premise:
It is assumed that you've at least read the primary handout on Moodle on the project. It is also assumed you have a complete understanding of CS180 Professor Mead's 
Memory Manager and CS280 Professor Volper's Memory Manager.

## Reminder Project Goal: 
The goal of the project is to create a memory debugger that can be easily dropped into a game. 
It should be able to detect all common types of heap related problems. Including leaks, over and underflows, mismatched allocators, and double frees. 
Depending on the type of error, the result should be a log file or an immediate break into the debugger so that the developer can easily track and solve these problems.


## Overview:
There are two main ways to complete this assignment, and they are dependent on how you hook into new()/delete(). The first way is to macro new()/delete() using the preprocessor. The second way is 
to overload new() and delete using C++. This document will show you how to get started using the non-macro method of overloading new()/delete().

|      | Macro                                                                                  | Overloaded                                                                     |   
|------|----------------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
| Pros | Easy to implement. Provides file and line of implementation No performance impact      | The 'right' way of doing it since it won't break C++ features.                 |                                                                                                           
| Cons | You lose out on all the benefits of C++ new() (placement new is unusable for instance) | How do you find the line file and function its called from? Performance Impact |   

The other methods are just variations of these two methods.


## Step 1: Start with the Mallocator


Seperate the responsibility of allocating memory and watching memory. 


### Problem:
The first problem you will encounter when writing a memory debugger is that using any STL data structure calls new()/delete(). 

#### Solution:
You can solve this by A) writing your own version of the STL data structure that uses malloc() or B) give the data structure an allocator through the template parameter.
C++ List: [Here](https://en.cppreference.com/w/cpp/container/list) 

Notice that in the third template argument you can pass in an allocator. 


For ease of use, I recommend using ["Mallocator"](https://devblogs.microsoft.com/cppblog/the-mallocator/) by MSVC's Visual C++ Team. Possible other methods include 
template specializing the STL's allocator. One of the sample projects might have one as well.

Note: All data structures in the memory debugger module should be using the custom allocator. Some of the memory techniques you will use for the debugger will bypass 
the mallocator.


## Step 2: Writing your basic debugger

Now we write a debugger that is the middleman between your Mallocator and the user. For simplicity, start with keeping track which memory is allocated and which is deallocated.
```c++
namespace MYCRT {
template<typename T>
using list = std::list<T, Mallocator<T>>;
}
... 
MyCRT::list<MemoryAddress> allocated_list; // private member of debugger class
...
// When allocating
MemoryAddress memory = malloc(size);
Debugger.Instance().ObserveMemory(memory);
return memory

// When deallocating
Debugger.Instance().MemoryDeallocated(memory);
... 
```

### Problem: 
Static initialization order of C++ objects is not guranteed. This will be explained more and solved in the next step.


## Step 3: Understanding how to hook your debugger into new/delete pairs.


For this method, you will hook into overloaded new()/delete(). Reference [here](https://en.cppreference.com/w/cpp/memory/new/operator_new).

```c++
// For MSVC
// Global overload 
void* operator new(size_t size);
void* operator new(size_t size, const std::nothrow_t&) noexcept;
void* operator new[](size_t size);
void* operator new[](size_t size, const std::nothrow_t&) noexcept;
void operator delete(void* address) noexcept;
void operator delete(void* address, size_t size) noexcept; 
void operator delete(void* address, const std::nothrow_t&) noexcept;
void operator delete[](void* address) noexcept;
void operator delete[](void* address, size_t size) noexcept;
void operator delete[](void* address, const std::nothrow_t&) noexcept;
```
You will have to overload each of these. Remember to make your code cross-compile in GCC and Clang!
Each function would probably look something like this
```c++
void* operator new(size_t size)
{
    MemoryAddress memory = VittualAlloc(...);
    // Debugger goes here to record information
    return memory;
}
```
Note: You can use the mallocator here if needed, but the technique for detecting buffer overflow/underflow will replace allocation.


### Problem 1: Integration of debugger. How will you allow the debugger to be easily called by your client?
#### Solution 1: 
Create global functions that route the user to the debugger.
#### Solution 2:
Make the debugger a singleton. <--- Recommended approach as you are causing ALL the memory called by new/delete to be under your debugger control.


### Problem 2: Static initialization and destruction order of C++ objects is not guranteed.
This is a nuanced problem. You have just overloaded your new/delete globablly, but what if your debugger is not initialized when it is called? What if the debugger is destroyed 
before a static std::vector is destroyed? (Fun fact: This happened in my GAM200). For example, if you had a static class that held a std:vector (much like your debugger), then
you would run into inconsistent problems when logging at the end of your program's lifetime!
The classic Singleton, Scott Meyer's Singleton, assures construction order.
```c++
inline Stream& getStream() { static Stream s; return s; }
static Stream& stream = getStream();
```
But it doesn't assure destruction order. 
#### Solution
The solution to this is to use the Nifty-Counter idiom. This assures construction AND destruction order.


[Wikibooks on Nifty Counter](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter)


[StackOverflow](https://stackoverflow.com/questions/36844393/c-nifty-counter-idiom-why)


It essentially adds a counter to your contruction and destruction times of a proxy object. My own implementation looked something like this
```c++
// Header file
struct MemoryDebuggerInitializer
{
	MemoryDebuggerInitializer();
	~MemoryDebuggerInitializer();
	static int s_count;
};

static MemoryDebuggerInitializer s_logInit;


// Constructor
MemoryDebuggerInitializer::MemoryDebuggerInitializer()
	if (++s_count == 1)
	{
		MemoryDebugger::s_instance = static_cast<MemoryDebugger*>(malloc(sizeof(MemoryDebugger)));
		MemoryDebugger::s_instance = new (MemoryDebugger::s_instance) MemoryDebugger; // Placement new
	}

// Destructor
MemoryDebuggerInitializer::~MemoryDebuggerInitializer()
{
	if (--s_count == 0)
	{
		MemoryDebugger::s_instance->~MemoryDebugger();
		free(MemoryDebugger::s_instance);
	}
}

// Fun fact, the wikibooks implementation of it is much better as it the memory is allocated as a static instead of a heap
```

## Step 4: Start adding features to your debugger!
At this point you should have a debugger module that is able to handle new/delete and track allocations (Don't do anything special with blocks or pages, just malloc and shove it 
into a list/vector), and it can be called from anywhere in the codebase since its a Singleton.

You can now start adding features to your debugger and start implementing memory debugging techniques. 

Tips:
  - For Windows, you need to use StackWalker to get the stack of the function call. This is in the "Sample - Stack Trace" project. Adapt that into your code base.
  - I used Professor Volper's technique of header files to store information in the memory ahead/behind the address.
    - I recommend implementing functions like Header& GetHeader(MemoryAddress ) and MEMTYPE& MemoryDebugger::GetMemoryType(MemoryAddress)
    - I used enums to keep track the if new/delete pair is called correctly, and slotted them into the headers.
    - Leave the read/write overflow last.





