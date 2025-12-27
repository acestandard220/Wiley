#pragma once
#include "Device.h"
#include "../Core/Utils.h"
#include "../Core/defines.h"
#include <queue>

#ifdef _DEBUG
	#define VALIDATE_DESCRIPTOR(descriptor,msg) if (!descriptor.valid){std::cout << "[ERROR] :: Invalid Descriptor. "<< msg << std::endl;}
#else 
	#define VALIDATE_DESCRIPTOR(descriptor,msg)
#endif 



namespace RHI
{
	template<typename T>
	using Queue = std::queue<T>;

	enum class DescriptorHeapType
	{
		RenderTarget = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		DepthTarget = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		ShaderResource = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,

		ComputeResource = ShaderResource
	};

	class DescriptorHeap
	{
	public:
		using Ref = std::shared_ptr<DescriptorHeap>;

		//Set of all heap types for convinience 
		struct Heap
		{
			DescriptorHeap::Ref rtv;
			DescriptorHeap::Ref dsv;
			DescriptorHeap::Ref cbv_srv_uav;
			DescriptorHeap::Ref sampler;
		};

		struct Descriptor
		{
			bool valid = true;
			UINT heapIndex = 0;

			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};

			DescriptorHeap* parentHeap = nullptr;

			Descriptor() = default;
			Descriptor(DescriptorHeap* descriptorHeap, int index);

			static Descriptor Invalid() {
				Descriptor descriptor;
				descriptor.valid = false;
				return descriptor;
			}
		};


		DescriptorHeap(Device::Ref device, DescriptorHeapType descriptorHeapType, UINT num = 1,const std::string& name = "Descritpor_Heap");
		~DescriptorHeap();

		/// <summary>
		///		Allocates a number of descriptors and sets all of them to null.
		///		Used by the resource cache so that directx does not complain about empty descriptors in the table.
		/// </summary>
		WILEY_NODISCARD std::vector<DescriptorHeap::Descriptor> AllocateNullDescriptor(uint32_t nDescriptors);

		WILEY_NODISCARD DescriptorHeap::Descriptor Allocate();
		WILEY_NODISCARD std::vector<DescriptorHeap::Descriptor> Allocate(UINT nDescriptors);

		void Deallocate(int index);

		WILEY_NODISCARD ID3D12DescriptorHeap* GetHeap() { return heap.Get(); }

		WILEY_NODISCARD D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int element);
		WILEY_NODISCARD D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int element);

		UINT GetIncrementSize()const { return incrementSize; }

	private:

		ComPtr<ID3D12DescriptorHeap> heap;
		Device::Ref _device;

		D3D12_DESCRIPTOR_HEAP_TYPE type;

		UINT numDescriptors;
		UINT descriptorPtr;
		UINT incrementSize;
		UINT heapSize;

		Queue<uint32_t> freelist;
	};
}

