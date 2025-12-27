#include "Buffer.h"

namespace RHI
{
    D3D12_HEAP_TYPE GetHeapType(BufferUsage usage)
    {
        switch (usage)
        {
            case BufferUsage::Index: [[fallthrough]];
            case BufferUsage::Vertex:[[fallthrough]];
            case BufferUsage::ShaderResource:
            case BufferUsage::PixelShaderResource:
            case BufferUsage::NonPixelShaderResource:
            case BufferUsage::ComputeStorage: return D3D12_HEAP_TYPE_DEFAULT;

            case BufferUsage::Copy: [[fallthrough]];
            case BufferUsage::Constant: return D3D12_HEAP_TYPE_UPLOAD;

            case BufferUsage::ReadBack: return D3D12_HEAP_TYPE_READBACK;
        }
    }

    D3D12_RESOURCE_FLAGS GetResourceFlag(BufferUsage usage)
    {
        switch (usage)
        {
            case BufferUsage::ComputeStorage:return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            default: return D3D12_RESOURCE_FLAG_NONE;
        }
    }

    Buffer::Buffer(Device::Ref device, BufferUsage usage, bool persistent, size_t size, uint64_t stride,
        const std::string& name, DescriptorHeap::Ref heap)
        : usage(usage), size(size), cBufferViewDesc({}), indexBufferView({}),vertexBufferView({})
    {
        CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size, GetResourceFlag(usage));
        CD3DX12_HEAP_PROPERTIES properties = CD3DX12_HEAP_PROPERTIES(GetHeapType(usage));

        switch (usage)
        {
            case BufferUsage::Constant:
                state = D3D12_RESOURCE_STATE_GENERIC_READ;
                break;
            default:
                state = D3D12_RESOURCE_STATE_COMMON;
                break;
        }

        HRESULT result = device->GetNative()->CreateCommittedResource(
            &properties, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&resource)
        );

        if (FAILED(result))
            std::cout << "Failed to create buffer resource.\n";

        if (usage == BufferUsage::Vertex) {
            vertexBufferView.BufferLocation = resource->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = static_cast<UINT>(size);
            vertexBufferView.StrideInBytes = static_cast<UINT>(stride);
        }
        else if (usage == BufferUsage::Index) {
            indexBufferView.BufferLocation = resource->GetGPUVirtualAddress();
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;
            indexBufferView.SizeInBytes = static_cast<UINT>(size);
        }
        else if (usage == BufferUsage::Constant) {
            // Align to 256 bytes
            UINT alignedSize = static_cast<UINT>((size + 255) & ~255);

            cBufferViewDesc.BufferLocation = resource->GetGPUVirtualAddress();
            cBufferViewDesc.SizeInBytes = alignedSize;

            if (heap)
            {
                cbv_uav = heap->Allocate();
                if (cbv_uav.valid)
                {
                    device->GetNative()->CreateConstantBufferView(
                        &cBufferViewDesc, cbv_uav.cpuHandle
                    );
                }
                else {
                    std::cout << "Invalid Descriptor Handle. Failed to create Constant Buffer View." << std::endl;
                }
            }
        }
        else if (usage == BufferUsage::ComputeStorage) {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
            desc.Format = DXGI_FORMAT_UNKNOWN;
            desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            desc.Buffer = {
                .FirstElement = 0,
                .NumElements = (stride) ? UINT(size / static_cast<size_t>(stride)) : UINT(size),
                .StructureByteStride = UINT(stride),
                .CounterOffsetInBytes = 0,
                .Flags = D3D12_BUFFER_UAV_FLAG_NONE,
            };

            if (heap)
            {
                cbv_uav = heap->Allocate();
                if (cbv_uav.valid)
                {
                    device->GetNative()->CreateUnorderedAccessView(
                        resource.Get(), nullptr, &desc, cbv_uav.cpuHandle
                    );
                }
                else {
                    std::cout << "Invalid Descriptor Handle. Failed to create Unordered Access View." << std::endl;
                }
            }
        }

        if(usage != BufferUsage::Constant)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            srvDesc.Buffer.NumElements = (stride) ? UINT(size / static_cast<size_t>(stride)) : UINT(size);
            srvDesc.Buffer.StructureByteStride = static_cast<size_t>(stride);
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

            if (heap)
            {
                srv = heap->Allocate();
                if (!srv.valid)
                {
                    std::cout << "Failed to allocate space for Buffer Shader Resource View." << std::endl;
                    return;
                }
                device->GetNative()->CreateShaderResourceView(resource.Get(), &srvDesc, srv.cpuHandle);
            }
        }

        WILEY_NAME_D3D12_OBJECT(resource, name);
    }


	Buffer::~Buffer()
	{
		resource.Reset();
	}

	void Buffer::Map(void** data, size_t begin, size_t end)
	{
        if (usage == BufferUsage::ComputeStorage || usage == BufferUsage::Index || usage == BufferUsage::Vertex)
        {
            std::cout << "Cannot map a resource with CPU access." << std::endl;
            return;
        }

		D3D12_RANGE range;
		range.Begin = begin;
		range.End = end;

		HRESULT result;
		if (end > begin)
			result = resource->Map(0, &range, data);
		else
			result = resource->Map(0, nullptr, data);
		if (FAILED(result))
			std::cout << "Failed to map buffer." << std::endl;
	}
	void Buffer::Unmap(size_t begin, size_t end)
	{
		D3D12_RANGE range;
		range.Begin = begin;
		range.End = end;

		if (end > begin)
			resource->Unmap(0, &range);
		else
			resource->Unmap(0, nullptr);
	}

    void Buffer::UploadData(void* data, size_t size, size_t start, size_t end)
    {
        UINT8* bufferPtr;
        Map(reinterpret_cast<void**>(&bufferPtr), start, end);
        memcpy(bufferPtr, data, size);
        Unmap(start, end);
    }

    void Buffer::UploadPersistent(void* dst, const void* src, size_t size)
    {
        memcpy(dst, src, size);
    }
    
    DescriptorHeap::Descriptor Buffer::GetSRV() const
    {
        if (usage == BufferUsage::Constant)
        {
            DescriptorHeap::Descriptor _descriptor;
            _descriptor.valid = false;
            return _descriptor;
        }
        return srv;
    }

    DescriptorHeap::Descriptor Buffer::GetCBV() const {
        if (usage != BufferUsage::Constant)
        {
            DescriptorHeap::Descriptor _descriptor;
            _descriptor.valid = false;
            return _descriptor;
        }
        return cbv_uav;
    }

    DescriptorHeap::Descriptor Buffer::GetUAV() const {
        if (usage != BufferUsage::ComputeStorage)
        {
            DescriptorHeap::Descriptor _descriptor;
            _descriptor.valid = false;
            return _descriptor;
        }
        return cbv_uav;
    }

    D3D12_VERTEX_BUFFER_VIEW* Buffer::GetVertexBufferView()
    {
        if (usage != BufferUsage::Vertex)
            return nullptr;
        return &vertexBufferView;
    }

    D3D12_INDEX_BUFFER_VIEW* Buffer::GetIndexBufferView()
    {
        if (usage != BufferUsage::Index)
            return nullptr;
        return &indexBufferView;
    }
}