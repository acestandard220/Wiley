#pragma once
//Created on 1/3/2026 00:04 ~

#include "Buffer.h"

#include "../Core/Allocator.h"

#include <memory>

namespace RHI
{

	/// <summary>
	///		This is a convience class around an RHI::Buffer and a LinearAllocator to allow for easy use of a CPU & GPU visible memory as a managed memory pool.
	///		The LinearAllocator owned by this class does not allocate any memory but only manages the memory from the Mapped resource Pointer.
	/// </summary>
	/// <typeparam name="T">
	///		T: the data type to be stored in this pool. The sizeof(T) defines the size of one block in this pool.
	/// </typeparam>

	template<typename T>
	class UploadBuffer : public Buffer
	{
		public:
	
			using Ref = std::shared_ptr<UploadBuffer>;

			/// <summary>
			///		Calls the constructor of RHI::Buffer with some fixed parameters to create and CPU & GPU visible memory pool.
			/// </summary>
			UploadBuffer(Device::Ref device, size_t size, uint32_t stride, const std::string name);
			~UploadBuffer();

			/// <summary>
			///		Resets the existing DirectX 12 resource and allocated a new committed resource with the newSize.
			///		All old contents of this buffer will be lost after the resize.
			/// </summary>
			/// <param name="newSize">
			///		newSize = oldCapacity + newExtension.
			///		newSize != newExtension only
			/// </param>
			void ResizeUploadBuffer(size_t newSize);

			Wiley::MemoryBlock<T> Allocate(int nElement);

			bool Deallocate(Wiley::MemoryBlock<T> memBlk);
			bool Deallocate(uint32_t index, uint32_t count);

			void Reset();

			T* GetBasePointer();
			T* GetTopPointer();

			size_t GetMemoryReach();
			size_t GetCapacity();

			T* GetPointerByIndex(uint32_t index);
			uint32_t GetIndexOffBasePointer(Wiley::MemoryBlock<T> memBlk);

		private:
			std::shared_ptr<Wiley::LinearAllocator<T>> memoryManager;
	};

	template<typename T>
	UploadBuffer<T>::UploadBuffer(Device::Ref device, size_t size, uint32_t stride, const std::string name)
		:Buffer(device, BufferUsage::Copy, true, size, stride, name, nullptr)
	{
		UINT8* bufferPtr = nullptr;
		Map(reinterpret_cast<void**>(&bufferPtr), 0, 0);
		memoryManager = std::make_shared<Wiley::LinearAllocator<T>>(size / stride, bufferPtr);
	}

	template<typename T>
	UploadBuffer<T>::~UploadBuffer()
	{
		Unmap(0, 0);
	}

	template<typename T>
	void UploadBuffer<T>::ResizeUploadBuffer(size_t newSize)
	{
		Unmap(0, 0);
		Resize(newSize);

		memoryManager.reset();

		UINT8* bufferPtr = nullptr;
		Map(reinterpret_cast<void**>(&bufferPtr), 0, 0);
		memoryManager = std::make_shared<Wiley::LinearAllocator<T>>(size / stride, bufferPtr);
	}

	template<typename T>
	Wiley::MemoryBlock<T> UploadBuffer<T>::Allocate(int nElement)
	{
		return memoryManager->Allocate(nElement);
	}

	template<typename T>
	bool UploadBuffer<T>::Deallocate(Wiley::MemoryBlock<T> memBlk)
	{
		return memoryManager->Deallocate(memBlk);
	}

	template<typename T>
	inline bool UploadBuffer<T>::Deallocate(uint32_t index, uint32_t count)
	{
		return memoryManager->Deallocate(index, count);
	}

	template<typename T>
	inline void UploadBuffer<T>::Reset()
	{
		memoryManager->Reset();
	}

	template<typename T>
	inline T* UploadBuffer<T>::GetBasePointer()
	{
		return (T*)memoryManager->GetBasePtr();
	}

	template<typename T>
	inline T* UploadBuffer<T>::GetTopPointer()
	{
		return (T*)memoryManager->GetTopPtr();
	}

	template<typename T>
	inline size_t UploadBuffer<T>::GetMemoryReach()
	{
		return memoryManager->GetReach();
	}

	template<typename T>
	inline size_t UploadBuffer<T>::GetCapacity()
	{
		return memoryManager->GetCapacity();
	}

	template<typename T>
	inline T* UploadBuffer<T>::GetPointerByIndex(uint32_t index)
	{
		return memoryManager->GetPointerByIndex(index);
	}

	template<typename T>
	inline uint32_t UploadBuffer<T>::GetIndexOffBasePointer(Wiley::MemoryBlock<T> memBlk)
	{
		return memoryManager->GetIndex(memBlk);
	}

}
