#include "CubeMap.h"

namespace RHI {

	CubeMap::CubeMap(Device::Ref device, TextureFormat format, uint32_t width, uint32_t height, DescriptorHeap::Heap& heap, const std::string& name)
		:_device(device), format(format), width(width), height(height), usage(TextureUsage::Common), creationState(TextureUsage::Common)
	{

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Height = height; 
		desc.Width = width;
		desc.DepthOrArraySize = 6;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT(format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

		HRESULT result = device->GetNative()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource));
		if (FAILED(result)) {
			WILEY_DEBUGBREAK;
			std::cout << "Failed to create cube map resource." << std::endl;
			return;
		}

		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT(format);
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			srv = heap.cbv_srv_uav->Allocate();
			if(srv.valid)
			{
				device->GetNative()->CreateShaderResourceView(resource.Get(), &srvDesc, srv.cpuHandle);
			}
		}

		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT(format);
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.ArraySize = 6;

			uav = heap.cbv_srv_uav->Allocate();
			if (uav.valid) {
				device->GetNative()->CreateUnorderedAccessView(resource.Get(), nullptr, &uavDesc, uav.cpuHandle);
			}
		}

		WILEY_NAME_D3D12_OBJECT(resource, name);
	}

	CubeMap::CubeMap(CubeMap&& other) noexcept
	{
		resource = other.resource;
		other.resource = nullptr;
	}

	CubeMap::~CubeMap()
	{
		resource.Reset();
		_device.reset();
	}

}