#pragma once

#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>  
#include <crtdbg.h> 
#include <windows.h>

namespace Wiley {

	class MemoryLeakDetector {
	public:

		static void SetMemoryAllocBreak(long allocID) {
			_CrtSetBreakAlloc(allocID);
		}

		static void DumpMemoryLeaks()
		{
			OutputDebugStringA("-----------_CrtDumpMemoryLeaks ---------\n");
			_CrtDumpMemoryLeaks();
		}

	private:

	};
}