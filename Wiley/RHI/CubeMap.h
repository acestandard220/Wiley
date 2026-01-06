#pragma once
//Created on 12/22/2025 11:15~

#include "Device.h"
#include "Texture.h"

namespace RHI {

	class CubeMap
	{
	public:
		using Ref = std::shared_ptr<CubeMap>;

		CubeMap(Device::Ref device, TextureFormat formate, uint32_t mapSize, int mips, DescriptorHeap::Heap& heap, const std::string& name = "CubeMap");
		CubeMap(CubeMap&& other) noexcept;
		~CubeMap();

		TextureUsage GetState()const { return usage; }
		void SetState(TextureUsage usag) { usage = usag; }
		
		TextureFormat GetFormat()const { return format; }

		uint32_t GetMapSize(int mip = 0)const;

		DescriptorHeap::Descriptor GetSRV()const { return srv; }
		DescriptorHeap::Descriptor GetUAV(int mip = 0)const;

		ID3D12Resource* GetResource()const { return resource.Get(); }

	private:
		Device::Ref _device;

		DescriptorHeap::Descriptor srv;

		std::vector<DescriptorHeap::Descriptor> uavs;

		TextureUsage usage;
		TextureUsage creationState;

		TextureFormat format;

		uint32_t mapSize;

		ComPtr<ID3D12Resource> resource;
	};


}