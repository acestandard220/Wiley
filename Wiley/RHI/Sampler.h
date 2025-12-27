#pragma once

//Created on 11/11/2025 at 18:13
#include "Device.h"
#include "DescriptorHeap.h"

namespace RHI
{
	enum class SamplerAddress
	{
		Wrap = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		Mirror = D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
		Clamp = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		Boarder = D3D12_TEXTURE_ADDRESS_MODE_BORDER
	};

	enum class SamplerFilter
	{
		Point = D3D12_FILTER_MIN_MAG_MIP_POINT,
		Linear = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		Anisotropy = D3D12_FILTER_ANISOTROPIC,

		ComparisonLinear = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR
	};

	class Sampler
	{
	public:
		using Ref = std::shared_ptr<Sampler>;

		Sampler(Device::Ref device, SamplerAddress address, SamplerFilter filter, DescriptorHeap::Heap& heap);
		~Sampler();

		DescriptorHeap::Descriptor GetDescriptor()const { return descriptor; }

	private:
		Device::Ref _device;
		DescriptorHeap::Descriptor descriptor;
	};

}
