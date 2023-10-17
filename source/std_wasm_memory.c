/*
File:   std_wasm_memory.c
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** Handles keeping track of the memory used by the web assembly module
	** Wasm memory is only allowed to grow, never shrink, and it grows in 4kB pages
*/

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
uint32_t WasmMemoryHeapStartAddress = 0;
uint32_t WasmMemoryHeapCurrentAddress = 0;
uint32_t WasmMemoryNumPagesAllocated = 0;

// +--------------------------------------------------------------+
// |                          Heap Base                           |
// +--------------------------------------------------------------+
extern unsigned char __heap_base;

__attribute__ ((visibility("default"))) 
void* GetHeapBasePntr()
{
	return &__heap_base;
}
// +==============================+
// |      GetHeapBaseAddress      |
// +==============================+
// uint32_t GetHeapBaseAddress()
WASM_EXPORTED_FUNC(uint32_t, GetHeapBaseAddress)
{
	void* basePntr = GetHeapBasePntr();
	return (uint32_t)basePntr;
}

// +--------------------------------------------------------------+
// |                        Main Functions                        |
// +--------------------------------------------------------------+
void InitWasmMemory(uint32_t numInitialPages)
{
	WasmMemoryNumPagesAllocated = numInitialPages;
	WasmMemoryHeapStartAddress = GetHeapBaseAddress();
	WasmMemoryHeapCurrentAddress = WasmMemoryHeapStartAddress;
}

void* WasmMemoryAllocate(uint32_t numBytes)
{
	uint32_t numPagesNeeded = (WasmMemoryHeapCurrentAddress + numBytes + 1) / WASM_MEMORY_PAGE_SIZE;
	if (numPagesNeeded > WASM_MEMORY_MAX_NUM_PAGES)
	{
		jsStdAbort("The WebAssembly module has run out of memory! WebAssembly only allows for 2GB of memory to be allocated per module", -1);
		return nullptr;
	}
	else if (WasmMemoryNumPagesAllocated < numPagesNeeded)
	{
		jsStdGrowMemory(numPagesNeeded - WasmMemoryNumPagesAllocated);
		WasmMemoryNumPagesAllocated = numPagesNeeded;
	}
	
	void* result = (void*)(WasmMemoryHeapCurrentAddress);
	WasmMemoryHeapCurrentAddress += numBytes;
	
	return result;
}
