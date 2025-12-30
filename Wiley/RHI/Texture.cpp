#include "Texture.h"

namespace RHI
{
	D3D12_RESOURCE_FLAGS GetResourceFlag(TextureUsage usage)
	{
		switch (usage)
		{
			case TextureUsage::DepthStencilTarget: return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			case TextureUsage::RenderTarget: [[fallthrough]];
			case TextureUsage::Present:return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			case TextureUsage::ComputeStorage:return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			case TextureUsage::ShaderResource:[[fallthrough]];
			default:return D3D12_RESOURCE_FLAG_NONE;
		}
	}

	DXGI_FORMAT GetDepthShaderResourceViewFormat(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::R32T: [[fallthrough]];
			case TextureFormat::D32:return DXGI_FORMAT_R32_FLOAT;
			case TextureFormat::D24S8:return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		}
	}
	DXGI_FORMAT GetDepthViewFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R32T:return DXGI_FORMAT_D32_FLOAT;
		default:return (DXGI_FORMAT)format;
		}
	}

	//Used by swapchain
	Texture::Texture(TextureUsage usage)
		:state(usage), creationState(usage), beforeState(usage), format(DXGI_FORMAT_UNKNOWN), width(0), height(0)
	{
	}

	Texture::Texture(Device::Ref device, TextureFormat fmt, UINT w, UINT h, TextureUsage usage, DescriptorHeap::Heap& heaps, const std::string& name)
		:_device(device), format((DXGI_FORMAT)fmt), width(w), height(h), creationState(usage), beforeState(usage)
	{
		state = usage;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Format = format;
		resourceDesc.MipLevels = 1;
		resourceDesc.Alignment = 0; 
		resourceDesc.DepthOrArraySize = 1; 
		resourceDesc.Height = height;
		resourceDesc.Width  = width;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Flags = GetResourceFlag(usage);

		D3D12_CLEAR_VALUE _clearValue{};
		{
			_clearValue.Color[0] = 1.0f;
			_clearValue.Color[1] = 1.0f;
			_clearValue.Color[2] = 1.0f;
			_clearValue.Color[3] = 1.0f;

			_clearValue.DepthStencil.Depth = 1.0f;
			_clearValue.DepthStencil.Stencil = 0;

			_clearValue.Format = GetDepthViewFormat(fmt);
		}

		auto _d3dDevice = device->GetNative();

		D3D12_CLEAR_VALUE* clearValue = &_clearValue;
		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		if (state == TextureUsage::RenderTarget || state == TextureUsage::Present || 
			state == TextureUsage::DepthStencilTarget) {
			clearValue = &_clearValue;
		}else {
			clearValue = nullptr;
		}

		HRESULT result = _d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, 
			&resourceDesc, (D3D12_RESOURCE_STATES)usage, clearValue, IID_PPV_ARGS(&resource));

		if (FAILED(result))
		{
			std::cout << "Failed to create texture resource." << std::endl;
			return;
		}

		BuildDescriptors(heaps);

#ifdef _DEBUG

		wchar_t lName[256];
		swprintf_s(lName, 256, L"%hs", name.c_str());
		resource->SetName(lName);

#endif // _DEBUG
	}

	Texture::Texture(Device::Ref device, UINT width, UINT height, DescriptorHeap::Heap& heaps, const std::string& name)
		:_device(device), width(width), height(height), state(TextureUsage::DepthStencilTarget), creationState(TextureUsage::DepthStencilTarget)
		,format(DXGI_FORMAT_D32_FLOAT)
	{
		auto _d3dDevice = device->GetNative();


		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Format = format;
		resourceDesc.MipLevels = 1;
		resourceDesc.Alignment = 0;
		resourceDesc.DepthOrArraySize = 6;
		resourceDesc.Height = height;
		resourceDesc.Width = width;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Flags = GetResourceFlag(state);

		D3D12_CLEAR_VALUE _clearValue{};
		{
			_clearValue.Color[0] = 1.0f;
			_clearValue.Color[1] = 1.0f;
			_clearValue.Color[2] = 1.0f;
			_clearValue.Color[3] = 1.0f;

			_clearValue.DepthStencil.Depth = 1.0f;
			_clearValue.DepthStencil.Stencil = 0;

			_clearValue.Format = GetDepthViewFormat(TextureFormat::D32);
		}

		D3D12_CLEAR_VALUE* clearValue = &_clearValue;

		if (state == TextureUsage::RenderTarget || state == TextureUsage::Present ||
			state == TextureUsage::DepthStencilTarget) {
			clearValue = &_clearValue;
		}
		else {
			clearValue = nullptr;
		}

		HRESULT result = _d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			(D3D12_RESOURCE_STATES)state, clearValue, IID_PPV_ARGS(&resource));
		if (FAILED(result))
		{
			std::cout << "Failed to create upload texture." << std::endl;
			return;
		}

		auto arrayDSV = heaps.dsv->Allocate(6);
		for (int i = 0; i < 6; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.MipSlice = 0;
			dsvDesc.Texture2DArray.FirstArraySlice = i; 
			dsvDesc.Texture2DArray.ArraySize = 1;

			shadowMapDSV[i] = arrayDSV[i];
			_d3dDevice->CreateDepthStencilView(resource.Get(), &dsvDesc, shadowMapDSV[i].cpuHandle);
		}

		WILEY_NAME_D3D12_OBJECT(resource, name);

	}


	RHI::Texture::Texture(Device::Ref device, TextureFormat fmt, UINT w, UINT h, TextureUsage usage, const std::string& name)
		:_device(device), format((DXGI_FORMAT)fmt), width(w), height(h), creationState(usage), beforeState(usage)
	{
		auto _d3dDevice = device->GetNative();
		state = usage;

		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Format = format;
		resourceDesc.MipLevels = 1;
		resourceDesc.Alignment = 0;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.Height = height;
		resourceDesc.Width = width;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Flags = GetResourceFlag(state);

		HRESULT result = _d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			(D3D12_RESOURCE_STATES)state, nullptr, IID_PPV_ARGS(&resource));
		if (FAILED(result))
		{
			std::cout << "Failed to create upload texture." << std::endl;
			return;
		}


#ifdef _DEBUG

		wchar_t lName[256];
		swprintf_s(lName, 256, L"%hs", name.c_str());
		resource->SetName(lName);

#endif // _DEBUG
	}

	Texture::~Texture()
	{
		if (rtv.parentHeap) {
			rtv.parentHeap->Deallocate(rtv.heapIndex);
		}
		if (dsv.parentHeap) {
			dsv.parentHeap->Deallocate(dsv.heapIndex);
		}
		if (uav.parentHeap) {
			uav.parentHeap->Deallocate(uav.heapIndex);
		}
		if (srv.parentHeap) {
			srv.parentHeap->Deallocate(srv.heapIndex);
		}

		resource.Reset();
		_device.reset();
	}

	Texture::Ref Texture::CreateUploadTexture(Device::Ref device, TextureFormat format, UINT width,
		UINT height, TextureUsage usage, const std::string& name)
	{
		return std::make_shared<Texture>(device, format, width, height, usage, name);
	}

	void Texture::Resize(UINT width, UINT height)
	{
		this->width = width;
		this->height = height;

		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		desc.Width = width;
		desc.Height = height;

		D3D12_HEAP_PROPERTIES heapProperties;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Color[0] = 1.0f;
		clearValue.Color[1] = 1.0f;
		clearValue.Color[2] = 1.0f;
		clearValue.Color[3] = 1.0f;

		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 1;

		clearValue.Format = format;
		D3D12_CLEAR_VALUE* _clearValue = &clearValue;

		if (creationState == TextureUsage::RenderTarget || creationState == TextureUsage::Present ||
			creationState == TextureUsage::DepthStencilTarget){
			_clearValue = &clearValue;
		}else {
			_clearValue = nullptr;
		}
		
		resource->GetHeapProperties(&heapProperties, nullptr);
		
		resource.Reset();

		_device->GetNative()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
			&desc, (D3D12_RESOURCE_STATES)creationState, nullptr, IID_PPV_ARGS(&resource));

		ID3D12Device* _d3dDevice = _device->GetNative();

		if (creationState == TextureUsage::DepthStencilTarget)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
			desc.Format = GetDepthViewFormat((TextureFormat)format);
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

			_d3dDevice->CreateDepthStencilView(resource.Get(), &desc, dsv.cpuHandle);
		}

		if (creationState == TextureUsage::RenderTarget || creationState == TextureUsage::Present)
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = format;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			_d3dDevice->CreateRenderTargetView(resource.Get(), &desc, rtv.cpuHandle);
		}

		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			if (creationState == TextureUsage::DepthStencilTarget)
				srvDesc.Format = GetDepthShaderResourceViewFormat((TextureFormat)format);
			else
				srvDesc.Format = format;

			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;

			_d3dDevice->CreateShaderResourceView(resource.Get(), &srvDesc, srv.cpuHandle);
		}

	}

	void Texture::BuildDescriptors(DescriptorHeap::Heap& heaps)
	{
		ID3D12Device* _d3dDevice = _device->GetNative();

		if (state == TextureUsage::DepthStencilTarget)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
			desc.Format = GetDepthViewFormat((TextureFormat)format);
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

			dsv = heaps.dsv->Allocate();
			_d3dDevice->CreateDepthStencilView(resource.Get(), &desc, dsv.cpuHandle);
		}

		//Might make this true for every resource type due to triansient resource aliasing.
		if (state == TextureUsage::RenderTarget || state == TextureUsage::Present)
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = format;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			rtv = heaps.rtv->Allocate();
			if (!rtv.valid)
			{
				std::cout << "Invalid Descritor Slot. Failed to allocate descriptor heap slot." << std::endl;
				
			}
			else {
				_d3dDevice->CreateRenderTargetView(resource.Get(), &desc, rtv.cpuHandle);
			}
		}

		if (state == TextureUsage::ComputeStorage)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
			desc.Format = format;
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 0;

			uav = heaps.cbv_srv_uav->Allocate();
			if (!uav.valid)
			{
				std::cout << "Invalid Descritor Slot. Failed to allocate descriptor heap slot." << std::endl;
			}
			else {
				_d3dDevice->CreateUnorderedAccessView(resource.Get(), nullptr, &desc, uav.cpuHandle);
			}
		}

		//Create a Shader Resource View for every texture resource
		//For Addtional Debugging/Visualization purposes if not used as one in the pipeline
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			if (state == TextureUsage::DepthStencilTarget)
				srvDesc.Format = GetDepthShaderResourceViewFormat((TextureFormat)format);
			else
				srvDesc.Format = format;

			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;

			srv = heaps.cbv_srv_uav->Allocate();
			_d3dDevice->CreateShaderResourceView(resource.Get(), &srvDesc, srv.cpuHandle);
		}
	}

	void Texture::BuildSRV(DescriptorHeap::Descriptor& srvDescriptor)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		if (state == TextureUsage::DepthStencilTarget)
			srvDesc.Format = GetDepthShaderResourceViewFormat((TextureFormat)format);
		else
			srvDesc.Format = format;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		_device->GetNative()->CreateShaderResourceView(resource.Get(), &srvDesc, srvDescriptor.cpuHandle);
		srv = srvDescriptor;
	}

	void Texture::Map(void** data, uint32_t start, uint32_t end)
	{
		CD3DX12_RANGE readRange(start, end);

		HRESULT result;
		if (end > start)
			result = resource->Map(0, &readRange, data);
		else
			result = resource->Map(0, nullptr, data);

		if (FAILED(result))
		{
			std::cout << "Failed to map Texture Resource. Ensure texture is a generic read.(was created with an upload heap)" 
				<< std::endl;
			return;
		}
	}

	void Texture::Unmap(uint32_t start, uint32_t end)
	{
		CD3DX12_RANGE readRange(start, end);

		if (end > start)
			resource->Unmap(0, &readRange);
		else
			resource->Unmap(0, nullptr);
	}

	void Texture::UploadData(void* data, size_t size, uint32_t start, uint32_t end)
	{
		UINT8* bufferPtr;
		Map(reinterpret_cast<void**>(&bufferPtr), start, end);
		memcpy(bufferPtr, data, size);
		Unmap(start, end);
	}

	void Texture::SetState(TextureUsage state) { 
		beforeState = this->state;
		this->state = state; 
	}

	void Texture::SetName(const std::string name)
	{
#ifdef _DEBUG
		WILEY_NAME_D3D12_OBJECT(resource, name);
#endif
	}

	TextureFormat Texture::GetTextureFormat(int bitPerChannel)
	{
		switch (bitPerChannel) {
			case 8:
				return TextureFormat::RGBA8_UNORM;
			case 16:
				return TextureFormat::RGBA16;
			case 32:
				return TextureFormat::RGBA32;
		}
	}

	const DescriptorHeap::Descriptor& Texture::GetDepthMapDSV(uint32_t index) const
	{
		return shadowMapDSV[index];
	}

	UINT Texture::GetBitPerChannel(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::R16_UNORM: [[fallthrough]];
			case TextureFormat::R16: [[fallthrough]];
			case TextureFormat::RG16: [[fallthrough]];
			case TextureFormat::RG16_UNORM: [[fallthrough]];
			case TextureFormat::RGBA16: [[fallthrough]];
			case TextureFormat::RGBA16_UNORM: return 16;

			case TextureFormat::R32: [[fallthrough]];
			case TextureFormat::R32T: [[fallthrough]];
			case TextureFormat::RG32: [[fallthrough]];
			case TextureFormat::RGB32: [[fallthrough]];
			case TextureFormat::RGBA32: return 32;
			
			default:return 8;
		}
	}

	UINT Texture::GetPixelSize(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::R16_UNORM: [[fallthrough]];
			case TextureFormat::R16: return sizeof(short);

			case TextureFormat::R32T: [[fallthrough]];
			case TextureFormat::R32:return sizeof(int);

			case TextureFormat::RG16_UNORM: [[fallthrough]];
			case TextureFormat::RG16: return 2.0f * sizeof(short);

			case TextureFormat::RG32: return 2.0f * sizeof(int);
			case TextureFormat::RGB32:return 3.0f * sizeof(int);

			case TextureFormat::RGBA16: return 4 * sizeof(short);
			case TextureFormat::RGBA32: return 4 * sizeof(int);
			case TextureFormat::RGBA8_UNORM: return 4 * sizeof(char);
		}
		return 0;
	}

}
