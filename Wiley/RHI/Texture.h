#pragma once
#include "Device.h"
#include "DescriptorHeap.h"

namespace RHI
{
	enum class TextureFormat
	{
		Unknown = DXGI_FORMAT_UNKNOWN,
		RG8_UNORM = DXGI_FORMAT_R8G8_UNORM,
		RGBA8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM,

		RGBA16_UNORM = DXGI_FORMAT_R16G16B16A16_UNORM,
		RG16_UNORM = DXGI_FORMAT_R16G16_UNORM,
		R16_UNORM = DXGI_FORMAT_R16_UNORM,

		RGBA16 = DXGI_FORMAT_R16G16B16A16_FLOAT,
		RG16 = DXGI_FORMAT_R16G16_FLOAT,
		R16 = DXGI_FORMAT_R16_FLOAT,

		//32-bits Complete
		RGBA32 = DXGI_FORMAT_R32G32B32A32_FLOAT,
		RGB32 = DXGI_FORMAT_R32G32B32_FLOAT,
		RG32 = DXGI_FORMAT_R32G32_FLOAT,
		R32 = DXGI_FORMAT_R32_FLOAT,

		D32 = DXGI_FORMAT_D32_FLOAT,
		D24S8 = DXGI_FORMAT_D24_UNORM_S8_UINT,

		R32T = DXGI_FORMAT_R32_TYPELESS,

		HDR10 = DXGI_FORMAT_R10G10B10A2_UNORM
	};

	enum class TextureUsage
	{
		DepthStencilTarget = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		DepthRead = D3D12_RESOURCE_STATE_DEPTH_READ,
		RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
		Present = D3D12_RESOURCE_STATE_PRESENT,
		ShaderResource = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		NonPixelShader = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		PixelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		Common = D3D12_RESOURCE_STATE_COMMON,


		CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
		CopySrc = D3D12_RESOURCE_STATE_COPY_SOURCE,
		GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,

		ComputeStorage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	};

	class Texture
	{
	public:
		using Ref = std::shared_ptr<Texture>;
		
		//Use when someone else is going to create the texture
		Texture(TextureUsage usage);
		
		//Normal use
		Texture(Device::Ref device,TextureFormat format, UINT width, UINT height,TextureUsage use, DescriptorHeap::Heap& heaps,const std::string& name = "Texture");

		//Used by ShadowMapManager
		Texture(Device::Ref device, UINT width, UINT height, DescriptorHeap::Heap& heaps, const std::string& name = "Texture");
		/// <summary>
		///		This constructor creates a texture resource without any views/descriptors to it.
		///		Used for pure shader resources/image textures.
		/// </summary>
		Texture(Device::Ref device, TextureFormat format, UINT width, UINT height, TextureUsage usage, const std::string& name = "Texture");
		~Texture();

		static Texture::Ref CreateUploadTexture(Device::Ref device, TextureFormat format, UINT width, UINT height, TextureUsage usage, const std::string& name = "Texture");

		void Resize(UINT width, UINT height);
		void BuildDescriptors(DescriptorHeap::Heap& heap);
		void BuildSRV(DescriptorHeap::Descriptor& srvDescriptor);

		void Map(void **data, uint32_t start, uint32_t end);
		void Unmap(uint32_t start, uint32_t end);

		//This function can only be used for upload heap textures.
		//Use a copy command queue to upload default textures.
		void UploadData(void* data, size_t size, uint32_t start, uint32_t end);

		void SetState(TextureUsage state);
		void SetName(const std::string name);

		static TextureFormat GetTextureFormat(int bitPerChannel);

		ComPtr<ID3D12Resource>& GetComPtr() { return resource; }
		ID3D12Resource* GetResource() { return resource.Get(); }
		ID3D12Resource** GetResourceAddress() { return resource.GetAddressOf(); }

		TextureFormat GetFormat() { return (TextureFormat)format; }
		TextureUsage GetState() { return state; }
		TextureUsage GetBeforeState() const { return beforeState; }

		UINT GetWidth()const { return width; }
		UINT GetHeight()const { return height; }

		const DescriptorHeap::Descriptor& GetRTVDescriptor()const { return rtv; }
		const DescriptorHeap::Descriptor& GetDSVDescriptor()const { return dsv; }
		const DescriptorHeap::Descriptor& GetSRV()const { return srv; }
		const DescriptorHeap::Descriptor& GetUAVDescriptor()const { return uav; }

		//DescriptorHeap::Descriptor CreateView(DescriptorHeapType type);
		
		//static TextureFormat GetTextureFormat(int nChannel, int bitPerChannel);
		static UINT GetBitPerChannel(TextureFormat format);
		static UINT GetPixelSize(TextureFormat format);

		bool operator==(const Texture& other)
		{
			return ((resource == other.resource) && 
				(width == other.width) && 
				(height == other.height));
		}

	private:
		friend class RenderContext;

		DescriptorHeap::Descriptor rtv;
		DescriptorHeap::Descriptor dsv;
		DescriptorHeap::Descriptor srv;
		DescriptorHeap::Descriptor uav;

		Device::Ref _device;
		ComPtr<ID3D12Resource> resource;

		//States
		TextureUsage state;
		TextureUsage beforeState;
		const TextureUsage creationState;

		DXGI_FORMAT format;
		UINT width;
		UINT height;
	};
}

