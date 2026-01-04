#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include "Utils.h"
#include "defines.h"

#include "Tracy/tracy/Tracy.hpp"

#include <memory>
#include <cstdlib>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <set>
#include <span>

namespace Wiley {

	template<typename T>
	using MemoryBlock = std::span<T>;

	//This struct is a generic representation of a memory block and is used by the freelist to make it indepent of where the memory sits incase of reallocations.
	struct MemoryBlockRaw {
		uint32_t offset;
		uint32_t size; //byte size
	};

	template<typename T>
	class LinearAllocator {
		class FreeList {
		public:
			struct SpanCompare {
				bool operator()(const MemoryBlockRaw& a, const MemoryBlockRaw& b) const {
					return a.offset < b.offset;
				}
			};

			void Optimize() {
				if (freelist.size() < 2) return;

				std::set<MemoryBlockRaw, SpanCompare> merged;
				auto it = freelist.begin();
				MemoryBlockRaw current = *it;
				++it;
				
				while (it != freelist.end()) {
					uint32_t currentEnd = current.offset + current.size;
					if (currentEnd == it->offset) {
						current = MemoryBlockRaw(current.offset, current.size + it->size);
					}
					else {
						merged.insert(current);
						current = *it;
					}
					++it;
				}
				merged.insert(current);
				freelist = std::move(merged);
			}

			std::set<MemoryBlockRaw, SpanCompare> freelist;
		};

	public:

		using value_type = T;
		using pointer = T*;
		using const_pointer = T const*;
		using reference = T&;
		using const_reference = T const&;

		LinearAllocator(uint32_t nElement)
			:nElement(nElement), capacity(sizeof(T)* nElement), used(0),
			elementSize(sizeof(T))
		{
			basePtr = nullptr;
			topPtr = nullptr;

			_init = false;
		};

		//This constructor is to be used to manager memory pools we have no control over allocation of. 
		//Example GPU Upload Heap memory

		LinearAllocator(uint32_t nElement, void* uploadHeapPtr)
			:nElement(nElement), capacity(sizeof(T) * nElement), used(0),
			elementSize(sizeof(T))
		{
			basePtr = uploadHeapPtr;
			topPtr = uploadHeapPtr;

			_init = true;
		}

		~LinearAllocator() {
			Free();
		}

		bool Initialize() {
			if (_init)
			{
				std::cout << "Attempting to reintialize memory allocator." << std::endl;
				return false;
			}

			basePtr = malloc(capacity);

			TracyAllocN(basePtr, capacity, "LinearAllocation");

			topPtr = basePtr;

			if (basePtr == nullptr) {
				std::cout << "Failed allocate memory." << std::endl;
				return false;
			}

			_init = true;
			return true;
		}

		bool Reallocate(uint32_t nElement)
		{
			uint32_t oldTopPtrOffset = (T*)topPtr - (T*)basePtr;

			void *newBasePtr = realloc(basePtr, elementSize * nElement);
			if (!newBasePtr)
				return false;

			basePtr = newBasePtr;
			topPtr = (T*)basePtr + oldTopPtrOffset;

			this->nElement = nElement;
			capacity = (sizeof(T) * nElement);

			return true;
		}

		void Free() {
			free(basePtr);
			TracyFreeN(basePtr, "LinearAllocation");
		}

		[[nodiscard]] MemoryBlock<T> Allocate(uint32_t nElement) {
			size_t reqSize = nElement * elementSize;

			for (auto it = freelist.freelist.begin(); it != freelist.freelist.end(); ++it) {
				if (it->size >= reqSize) {
					uint32_t ptr = it->offset;
					if (it->size == reqSize) {
						freelist.freelist.erase(it);
					}
					else {
						MemoryBlockRaw modified = MemoryBlockRaw(it->offset + reqSize, it->size - reqSize);
						freelist.freelist.erase(it);
						freelist.freelist.insert(modified);
					}
					return MemoryBlock<T>((T*)basePtr + ptr, static_cast<uint32_t>(reqSize));
				}
			}

			if (used + reqSize > capacity) {
				if (!Reallocate(this->nElement + (2 * nElement))) {
					WILEY_DEBUGBREAK;
					std::cout << "Not enough memory for allocation." << std::endl;
					return { (T*)topPtr, 0 };
				}
			}

			void* returnPtr = topPtr;
			topPtr = (uint8_t*)topPtr + reqSize;
			used += reqSize;
			MemoryBlock<T> returnSpan((T*)returnPtr, static_cast<uint32_t>(nElement));
			return returnSpan;
		}

		[[nodiscard]] bool Deallocate(pointer blockPtr, uint32_t nElement)
		{
			if (blockPtr > topPtr) {
				std::cout << "Attempting to free invalid memory block." << std::endl;
				return false;
			}

			if ((blockPtr + (nElement * elementSize)) == topPtr) {
				topPtr = topPtr - (nElement * elementSize);
				return true;
			}

			MemoryBlock<T> freeBlock(blockPtr, nElement);
			freelist.freelist.insert(freeBlock);
			freelist.Optimize();
		}

		[[nodiscard]] bool Deallocate(MemoryBlock<T> block)
		{
			if (block.data() > topPtr) {
				std::cout << "Attempting to free invalid memory block." << std::endl;
				return false;
			}

			if ((uint8_t*)(block.data() + block.size()) == topPtr){
				topPtr = (T*)topPtr - (nElement * elementSize);
				return true;
			}

			MemoryBlockRaw blkRaw(block.data() - basePtr, block.size_bytes());
			freelist.freelist.insert(blkRaw);
			freelist.Optimize();
		}

		[[nodiscard]] bool Deallocate(uint32_t index, uint32_t count) {
			auto _ptr = (T*)basePtr + index;
			MemoryBlock<T> memBlk( _ptr, count );
			return Deallocate(memBlk);
		}

		void Reset(){
			freelist.freelist.clear();
			topPtr = basePtr;
			used = 0;
		}

			[[nodiscard]] void* GetBasePtr()const {
				return basePtr;
			}

			[[nodiscard]] void* GetTopPtr()const {
				return topPtr;
			}

			[[nodiscard]] size_t GetMemoryUsed()const {
				return used;
			}
			 
			[[nodiscard]] size_t GetCapacity()const {
				return capacity;
			}

			[[nodiscard]] size_t GetRemainingMemory()const {
				return capacity - used;
			}

			[[nodiscard]] size_t GetReach()const {
				return (uint8_t*)topPtr - (uint8_t*)basePtr;
			}

			[[nodiscard]] uint32_t GetIndex(MemoryBlock<T> memBlk)const {
				return memBlk.data() - (T*)basePtr;
			}
			
			[[nodiscard]] T* GetPointerByIndex(uint32_t index)const {
				return (T*)basePtr + index;
			}

		private:
			void* basePtr;
			void* topPtr;

			uint32_t nElement;
			size_t elementSize;
			size_t capacity;
			size_t used;

			bool _init;
			FreeList freelist;
	};

}

#endif // _ALLOCATOR_H