#include "Sampler.h"

namespace RHI
{

	Sampler::Sampler(Device::Ref device, SamplerAddress address, SamplerFilter filter, SamplerComparisonFunc compFunc, float maxAni, DescriptorHeap::Heap& heap)
		:_device(device)
	{
		D3D12_SAMPLER_DESC desc{};
		desc.Filter = (D3D12_FILTER)filter;

		desc.AddressU = (D3D12_TEXTURE_ADDRESS_MODE)address;
		desc.AddressV = (D3D12_TEXTURE_ADDRESS_MODE)address;
		desc.AddressW = (D3D12_TEXTURE_ADDRESS_MODE)address;

		desc.MaxAnisotropy = maxAni;
		desc.ComparisonFunc = (D3D12_COMPARISON_FUNC)compFunc;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D12_FLOAT32_MAX;
		desc.MipLODBias = 0.0f;
		desc.BorderColor[0] = 1.0f;
		desc.BorderColor[1] = 1.0f;
		desc.BorderColor[2] = 1.0f;
		desc.BorderColor[3] = 1.0f;
		
		descriptor = heap.sampler->Allocate();

		device->GetNative()->CreateSampler(&desc, descriptor.cpuHandle);
	}

	Sampler::~Sampler()
	{

	}

}