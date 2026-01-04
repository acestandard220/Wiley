#pragma once
#include "../Core/defines.h"

#include "Device.h"
#include "DescriptorHeap.h"

#include <span>

namespace RHI
{
	enum class BufferFormat
	{
		Float_16 = DXGI_FORMAT_R16_FLOAT,
		Float2_16 = DXGI_FORMAT_R16G16_FLOAT,
		Float4_16 = DXGI_FORMAT_R16G16B16A16_FLOAT,

		Float_32 = DXGI_FORMAT_R32_FLOAT,
		Float2_32 = DXGI_FORMAT_R32G32_FLOAT,
		Float3_32 = DXGI_FORMAT_R32G32B32_FLOAT,
		Float4_32 = DXGI_FORMAT_R32G32B32A32_FLOAT
	};

	

	enum class BufferUsage
	{
		Constant = D3D12_RESOURCE_STATE_GENERIC_READ,
		Vertex = D3D12_RESOURCE_STATE_COMMON,
		Index = D3D12_RESOURCE_STATE_INDEX_BUFFER,
		ComputeStorage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		Copy = D3D12_RESOURCE_STATE_COPY_SOURCE,
		ShaderResource = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		NonPixelShaderResource = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		PixelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		ReadBack = D3D12_RESOURCE_STATE_COPY_DEST
	};

	class Buffer
	{
	public:
		using Ref = std::shared_ptr<Buffer>;

		Buffer(BufferUsage usage, UINT64 size)
			:size(size), usage(usage), cBufferViewDesc({}), indexBufferView({}), vertexBufferView({})
		{
		}

		Buffer(Device::Ref device, BufferUsage usage, bool persistent, size_t size, uint64_t stride, const std::string& name,
			DescriptorHeap::Ref descriptor = nullptr);
		~Buffer();

		void Resize(size_t newSize);

		void Map(void** data, size_t begin, size_t end);
		void Unmap(size_t begin, size_t end);

		//Slowly Removing...
		void UploadData(void* data, size_t size, size_t start, size_t end);
		void UploadPersistent(void* dst, const void* src, size_t size);

		//Use these instead...
		template<typename T> 
		void UploadData(const std::span<T>& src) {
#ifdef _DEBUG
			D3D12_HEAP_PROPERTIES heapProperties{};
			D3D12_HEAP_FLAGS heapFlag;
			resource->GetHeapProperties(&heapProperties, &heapFlag);

			if (heapProperties.Type == D3D12_HEAP_TYPE_DEFAULT) {
				WILEY_DEBUGBREAK;
				std::cout << "Cannot write to a default heap." << std::endl;
			}
#endif // DEBUG

			T* bufferPtr;
			Map(reinterpret_cast<void**>(&bufferPtr), 0, 0);
			memcpy(bufferPtr, src.data(), src.size_bytes());
			Unmap(0, 0);
		}

		template<typename T>
			requires std::is_trivially_copyable_v<T>
		void ReadData(std::span<T> dst) {

#ifdef _DEBUG
			D3D12_HEAP_PROPERTIES heapProperties{};
			D3D12_HEAP_FLAGS heapFlag;
			resource->GetHeapProperties(&heapProperties, &heapFlag);

			if (heapProperties.Type == D3D12_HEAP_TYPE_DEFAULT) {
				WILEY_DEBUGBREAK;
				std::cout << "Cannot read from a default heap." << std::endl;
			}

			if (heapProperties.Type == D3D12_HEAP_TYPE_UPLOAD) {
				std::cout << "Reading from upload heap might be slow. Consider read from a readback buffer." << std::endl;
			}
#endif // DEBUG

			T* bufferPtr;
			Map(reinterpret_cast<void**>(&bufferPtr), 0, dst.size_bytes());
			memcpy(dst.data(), bufferPtr, dst.size_bytes());
			Unmap(0, 0);
		}	

		ID3D12Resource* GetResource() const;
		ID3D12Resource** GetResourceAddress();

		void SetState(D3D12_RESOURCE_STATES newState);

		DescriptorHeap::Descriptor GetSRV() const;
		DescriptorHeap::Descriptor GetCBV() const;
		DescriptorHeap::Descriptor GetUAV() const;

		D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView();

		D3D12_RESOURCE_STATES GetState()const;

		size_t GetSize()const;
		uint32_t GetStride()const;
		BufferUsage GetUsage()const;

	protected:
		Device::Ref _device;
		ComPtr<ID3D12Resource> resource;

		size_t size;
		uint32_t stride;

		BufferUsage usage;
		D3D12_RESOURCE_STATES state;

		DescriptorHeap::Ref parentHeap;
		DescriptorHeap::Descriptor cbv_uav;
		DescriptorHeap::Descriptor srv;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cBufferViewDesc;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
	};

}