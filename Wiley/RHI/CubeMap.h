#pragma once
//Created on 12/22/2025 11:15~

#include "Device.h"
#include "Texture.h"

namespace RHI {

	class CubeMap
	{
	public:
		using Ref = std::shared_ptr<CubeMap>;

		CubeMap(Device::Ref device, TextureFormat formate, uint32_t width, uint32_t height, DescriptorHeap::Heap& heap, const std::string& name = "CubeMap");
		CubeMap(CubeMap&& other) noexcept;
		~CubeMap();

		TextureUsage GetState()const { return usage; }
		void SetState(TextureUsage usag) { usage = usag; }

		uint32_t GetWidth()const { return width; }
		uint32_t GetHeight()const { return height; }
		TextureFormat GetFormat()const { return format; }

		DescriptorHeap::Descriptor GetSRV()const { return srv; }
		DescriptorHeap::Descriptor GetUAV()const { return uav; }

		ID3D12Resource* GetResource()const { return resource.Get(); }

	private:
		Device::Ref _device;

		DescriptorHeap::Descriptor srv;
		DescriptorHeap::Descriptor uav;

		TextureUsage usage;
		TextureUsage creationState;

		TextureFormat format;
		uint32_t width;
		uint32_t height;

		ComPtr<ID3D12Resource> resource;
	};


}