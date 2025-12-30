#include "DescriptorHeap.h"

namespace RHI
{
	const char* DescriptorTypeString(DescriptorHeapType type)
	{
		switch (type)
		{
			case DescriptorHeapType::ShaderResource:return "D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER";
			case DescriptorHeapType::RenderTarget:return "D3D12_DESCRIPTOR_HEAP_TYPE_RTV";
			case DescriptorHeapType::DepthTarget: return "D3D12_DESCRIPTOR_HEAP_TYPE_DSV";
			case DescriptorHeapType::Sampler:return "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV";
			default:return "Unknown Descriptor Type";
		}
	}

	DescriptorHeap::DescriptorHeap(Device::Ref device, DescriptorHeapType descriptorHeapType, UINT num, const std::string& name)
		:_device(device), type((D3D12_DESCRIPTOR_HEAP_TYPE)descriptorHeapType), numDescriptors(num), descriptorPtr(0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = num;

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		else
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			
		heapDesc.Type = type;

		HRESULT result = device->GetNative()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));
		if (FAILED(result))
		{
			std::cout << "Failed to create descriptor heap of type " << 
				DescriptorTypeString(descriptorHeapType) << std::endl;
		}

		incrementSize = device->GetNative()->GetDescriptorHandleIncrementSize(type);
		heapSize = incrementSize * num;		

		WILEY_NAME_D3D12_OBJECT(heap, name);
	}

	DescriptorHeap::~DescriptorHeap()
	{
		heap.Reset();
		_device.reset();
	}

	std::vector<DescriptorHeap::Descriptor> DescriptorHeap::AllocateNullDescriptor(uint32_t nDescriptors) {

		std::vector<DescriptorHeap::Descriptor> descriptors(nDescriptors);
		for (int i = 0; i < nDescriptors; i++) {
			descriptors[i] = Allocate();

			switch (this->type) {
				case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
					nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					nullSrvDesc.Texture2D.MipLevels = 1;

					_device->GetNative()->CreateShaderResourceView(nullptr, &nullSrvDesc, descriptors[i].cpuHandle);
					break;
				}
				case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
				{
					D3D12_DEPTH_STENCIL_VIEW_DESC nullDsvDesc{};
					nullDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
					nullDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					
					_device->GetNative()->CreateDepthStencilView(nullptr, &nullDsvDesc, descriptors[i].cpuHandle);
					break;
				}
				case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
				{
					D3D12_RENDER_TARGET_VIEW_DESC nullRtvDesc{};
					nullRtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					nullRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

					_device->GetNative()->CreateRenderTargetView(nullptr, &nullRtvDesc, descriptors[i].cpuHandle);
					break;
				}
			}
		}
		return descriptors;
	}

	DescriptorHeap::Descriptor DescriptorHeap::Allocate()
	{
		if (descriptorPtr >= numDescriptors)
		{
			std::cout << "[ERROR] :: Descriptor Heap has no space for addition Descriptor." << std::endl;
			std::cout << "[ERROR] :: Trying to access out of bounds Descriptor Heap memory." << std::endl;

			DescriptorHeap::Descriptor descriptor{};
			descriptor.valid = false;
			descriptor.parentHeap = nullptr;
			return descriptor;
		}

		uint32_t index;
		if (freelist.size()) {
			index = freelist.front();
			freelist.pop();
		}
		else {
			index = descriptorPtr++;
		}

		DescriptorHeap::Descriptor dscriptor(this, index);
		return dscriptor;
	}

	//Todo: Error check for overflow...
	WILEY_NODISCARD std::vector<DescriptorHeap::Descriptor> DescriptorHeap::Allocate(UINT nDescriptors)
	{
		std::vector<DescriptorHeap::Descriptor> descriptors(nDescriptors);
		for (int i = 0; i < nDescriptors; i++) {
			descriptors[i] = Allocate();
		}
		return descriptors;
	}

	void DescriptorHeap::Deallocate(int index)
	{
		if (index > numDescriptors)
			return;

		if (index == descriptorPtr - 1) {
			descriptorPtr--;
			return;
		}

		freelist.push(index);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle(int element)
	{
		if (element * incrementSize > heapSize)
		{
			std::cout << "Failed to get descriptor handle at element " << element << std::endl;
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), element, incrementSize);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(int element)
	{
		if (element * incrementSize > heapSize)
		{
			std::cout << "Failed to get descriptor handle at element " << element << std::endl;
			return CD3DX12_GPU_DESCRIPTOR_HANDLE();
		}

		return CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), element, incrementSize);
	}

	DescriptorHeap::Descriptor::Descriptor(DescriptorHeap* descriptorHeap, int index)
	{
		parentHeap = descriptorHeap;
		heapIndex = index;

		cpuHandle = descriptorHeap->GetCPUHandle(index);

		if (descriptorHeap->type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || 
			descriptorHeap->type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
			gpuHandle = descriptorHeap->GetGPUHandle(index);
	}
}
