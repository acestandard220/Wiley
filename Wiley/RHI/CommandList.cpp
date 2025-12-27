#include "CommandList.h"
#include "Tracy/tracy/Tracy.hpp"

namespace RHI
{
	CommandList::CommandList(Device::Ref device, CommandListType commandListType, const std::string& name)
		:_device(device), type((D3D12_COMMAND_LIST_TYPE)commandListType)
	{
		HRESULT result;

		result = device->GetNative()->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator));
		if (FAILED(result))
		{
			std::cout << "Failed to create command allocator\n";
		}
		result = device->GetNative()->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
		if (FAILED(result))
		{
			std::cout << "Failed to create command list\n";
		}
		commandList->Close();

#ifdef _DEBUG
		wchar_t lName[256];
		swprintf_s(lName, 256, L"%hs", name.c_str());
		commandList->SetName(lName);

		std::string allocName = name + "_Allocator";
		wchar_t lName2[256];
		swprintf_s(lName2, 256, L"%hs", allocName.c_str());
		commandAllocator->SetName(lName2);
#endif // _DEBUG

	}

	CommandList::~CommandList()
	{
		commandList.Reset();
		commandAllocator.Reset();
		_device.reset();
	}

	CommandList::Ref CommandList::CreateCommandList(Device::Ref device, CommandListType commandListType, const std::string& name)
	{
		return std::make_shared<CommandList>(device, commandListType, name);
	}

	void CommandList::Continue(const std::vector<DescriptorHeap::Ref>& heap)
	{
		std::vector<ID3D12DescriptorHeap*> ppHeaps;
		for (int i = 0; i < heap.size(); i++)
			ppHeaps.push_back((heap[i]->GetHeap()));

		if (heap.size())
			commandList->SetDescriptorHeaps(heap.size(), ppHeaps.data());
	}

	void CommandList::Begin(const std::vector<DescriptorHeap::Ref>& heap)
	{
		ZoneScopedN("CommandList::Begin");

		commandAllocator->Reset();
		commandList->Reset(commandAllocator.Get(), nullptr);

		std::vector<ID3D12DescriptorHeap*> ppHeaps;
		for (int i = 0; i < heap.size(); i++)
			ppHeaps.push_back((heap[i]->GetHeap()));

		if(heap.size())
			commandList->SetDescriptorHeaps(heap.size(), ppHeaps.data());
	}

	void CommandList::End()
	{
		ZoneScopedN("CommandList::End");

		commandList->Close();
	}

	void CommandList::ImageBarrier(Texture::Ref texture, TextureUsage newState)
	{
		TextureUsage oldState = texture->GetState();
		if (oldState == newState)
			return;

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->GetResource(),
			(D3D12_RESOURCE_STATES)oldState, (D3D12_RESOURCE_STATES)newState);

		commandList->ResourceBarrier(1, &barrier);
		texture->SetState(newState);
	}

	void CommandList::ImageBarrier(std::vector<Barrier> barriers)
	{
		std::vector<CD3DX12_RESOURCE_BARRIER> resourceBarrier;
		for (int i = 0; i < barriers.size(); i++)
		{
			Barrier& bRef = barriers[i];

			TextureUsage oldState = bRef.texture->GetState();
			if (oldState == bRef.newState)
				continue;

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(bRef.texture->GetResource(),
				(D3D12_RESOURCE_STATES)oldState, (D3D12_RESOURCE_STATES)bRef.newState);
			
			bRef.texture->SetState(bRef.newState);
			resourceBarrier.push_back(barrier);
		}

		if (!resourceBarrier.size())
			return;

		commandList->ResourceBarrier(resourceBarrier.size(), resourceBarrier.data());
	}

	void CommandList::BufferBarrier(Buffer::Ref buffer, D3D12_RESOURCE_STATES usage)
	{
		D3D12_RESOURCE_STATES oldState = buffer->GetState();
		if (oldState == usage)
			return;

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer->GetResource(),
			(D3D12_RESOURCE_STATES)oldState, (D3D12_RESOURCE_STATES)usage);

		commandList->ResourceBarrier(1, &barrier);
		buffer->SetState(usage);
	}

	void CommandList::CubeMapBarrier(CubeMap::Ref buffer, D3D12_RESOURCE_STATES usage)
	{
		D3D12_RESOURCE_STATES oldState = D3D12_RESOURCE_STATES(buffer->GetState());
		if (oldState == usage)
			return;

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer->GetResource(),
			(D3D12_RESOURCE_STATES)oldState, (D3D12_RESOURCE_STATES)usage);

		commandList->ResourceBarrier(1, &barrier);
		buffer->SetState(TextureUsage(usage));
	}

	void CommandList::BufferUAVToCopySource(Buffer::Ref buffer)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->GetResource(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BufferCopySourceToUAV(Buffer::Ref buffer)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->GetResource(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BufferUAVToCopyDest(Buffer::Ref buffer)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->GetResource(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BufferCopyDestToUAV(Buffer::Ref buffer)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->GetResource(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BufferUAVToNonPixelShader(const std::vector<Buffer::Ref>& buffers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(buffers.size());
		for (int i = 0; i < buffers.size(); i++) {
			const Buffer::Ref& buffer = buffers[i];
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				buffer->GetResource(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			);
		}
		commandList->ResourceBarrier(buffers.size(), barriers.data());
	}

	void CommandList::BufferNonPixelShaderToUAV(const std::vector<Buffer::Ref>& buffers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(buffers.size());
		for (int i = 0; i < buffers.size(); i++) {
			const Buffer::Ref& buffer = buffers[i];
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				buffer->GetResource(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			);
		}
		commandList->ResourceBarrier(buffers.size(), barriers.data());
	}

	void CommandList::BufferUAVToPixelShader(const std::vector<Buffer::Ref>& buffers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(buffers.size());
		for (int i = 0; i < buffers.size(); i++) {
			const Buffer::Ref& buffer = buffers[i];
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				buffer->GetResource(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			);
		}
		commandList->ResourceBarrier(buffers.size(), barriers.data());
	}

	void CommandList::BufferPixelShaderToUAV(const std::vector<Buffer::Ref>& buffers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(buffers.size());
		for (int i = 0; i < buffers.size(); i++) {
			const Buffer::Ref& buffer = buffers[i];
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				buffer->GetResource(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			);
		}
		commandList->ResourceBarrier(buffers.size(), barriers.data());
	}

	void CommandList::BufferUAVToAllShader(Buffer::Ref buffer)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->GetResource(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BufferAllShaderToUAV(Buffer::Ref buffer)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->GetResource(),
			D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BufferUAVToCommon(const std::vector<Buffer*>& buffers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(buffers.size());
		for (int i = 0; i < buffers.size(); i++) {
			const Buffer* buffer = buffers[i];
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				buffer->GetResource(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COMMON
			);
		}
		commandList->ResourceBarrier(buffers.size(), barriers.data());
	}

	void CommandList::BufferCommonToUAV(const std::vector<Buffer*>& buffers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(buffers.size());
		for (int i = 0; i < buffers.size(); i++) {
			const Buffer* buffer = buffers[i];
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				buffer->GetResource(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			);
		}
		commandList->ResourceBarrier(buffers.size(), barriers.data());
	}

	void CommandList::UAVBarrier(Buffer::Ref buffer)
	{
		D3D12_RESOURCE_BARRIER uavBarrier;
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = buffer->GetResource();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		commandList->ResourceBarrier(1, &uavBarrier);
	}

	void CommandList::ImageBarrierSingles(std::vector<Barrier> barriers)
	{
		for (int i = 0; i < barriers.size(); i++)
		{
			Barrier& bRef = barriers[i];

			TextureUsage oldState = bRef.texture->GetState();
			if (oldState == bRef.newState)
				continue;

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(bRef.texture->GetResource(),
				(D3D12_RESOURCE_STATES)oldState, (D3D12_RESOURCE_STATES)bRef.newState);

			bRef.texture->SetState(bRef.newState);
			commandList->ResourceBarrier(1, &barrier);
		}
	}

	void CommandList::SetGraphicsPipeline(GraphicsPipeline::Ref graphicsPipeline)
	{
		commandList->SetPipelineState(graphicsPipeline->GetNative());
	}

	void CommandList::SetComputePipeline(ComputePipeline::Ref computePipeline)
	{
		commandList->SetPipelineState(computePipeline->GetNative());
	}

	void CommandList::SetGraphicsRootSignature(RootSignature::Ref rootSignature)
	{
		commandList->SetGraphicsRootSignature(rootSignature->GetNative());
	}

	void CommandList::SetComputeRootSignature(RootSignature::Ref rootSignature)
	{
		commandList->SetComputeRootSignature(rootSignature->GetNative());
	}

	void CommandList::BindShaderResource(DescriptorHeap::Descriptor descriptor, int index)
	{
		VALIDATE_DESCRIPTOR(descriptor,"Cannot bind invalid descriptor as Shader Resource")
		commandList->SetGraphicsRootDescriptorTable(index, descriptor.gpuHandle);
	}

	void CommandList::BindSamplerResource(DescriptorHeap::Descriptor descriptor, int index)
	{
		VALIDATE_DESCRIPTOR(descriptor,"Cannot bind invalid descriptor as Sampler Resource")
		commandList->SetGraphicsRootDescriptorTable(index, descriptor.gpuHandle);
	}

	void CommandList::PushConstant(const void* data, size_t size, uint32_t index)
	{
		commandList->SetGraphicsRoot32BitConstants(index, size / 4, data, 0);
	}

	void CommandList::BindComputeShaderResource(DescriptorHeap::Descriptor descriptor, int index) {
		VALIDATE_DESCRIPTOR(descriptor, "Cannot bind invalid descriptor as Compute Shader Resource");
		commandList->SetComputeRootDescriptorTable(index, descriptor.gpuHandle);
	}

	void CommandList::PushComputeConstant(const void* data, size_t size, uint32_t index)
	{
		commandList->SetComputeRoot32BitConstants(index, size / 4, data, 0);
	}

	void CommandList::BindComputeSamplerResource(DescriptorHeap::Descriptor descriptor, int index)
	{
		VALIDATE_DESCRIPTOR(descriptor, "Cannot bind invalid descriptor as Compute Shader Sampler");
		commandList->SetComputeRootDescriptorTable(index, descriptor.gpuHandle);
	}

	void CommandList::BindConstantBuffer(DescriptorHeap::Descriptor descriptor, int index)
	{
		VALIDATE_DESCRIPTOR(descriptor, "Cannot bind invalid descriptor as Constant Buffer")
		commandList->SetGraphicsRootDescriptorTable(index, descriptor.gpuHandle);
	}

	void CommandList::BindVertexBuffer(Buffer::Ref buffer)
	{
		auto view = buffer->GetVertexBufferView();
#ifdef _DEBUG

		if (view == nullptr)
		{
			std::cout << "Buffer provided is not a vertex buffer.\n" <<
				std::endl;
			return;
		}
#endif
		commandList->IASetVertexBuffers(0, 1, view);
	}

	void CommandList::BindIndexBuffer(Buffer::Ref buffer)
	{
		auto view = buffer->GetIndexBufferView();
#if _DEBUG
		if (view == nullptr)
		{
			std::cout << "Buffer provided is not a index buffer.\n" <<
				std::endl;
			return;
		}

#endif 
		commandList->IASetIndexBuffer(view);
	}

	void CommandList::SetDescriptors(const std::vector<DescriptorHeap::Ref>& heaps)
	{
		std::vector<ID3D12DescriptorHeap*> ppHeaps;
		for (auto& heap : heaps)
			ppHeaps.push_back(heap->GetHeap());

		commandList->SetDescriptorHeaps(ppHeaps.size(), ppHeaps.data());
	}

	void CommandList::SetRenderTargets(const std::vector<DescriptorHeap::Descriptor>& rtvDescriptors, DescriptorHeap::Descriptor dsvDescriptor)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
		D3D12_CPU_DESCRIPTOR_HANDLE dsv;

		for (int i = 0; i < rtvDescriptors.size(); i++)
			rtvHandles.push_back(rtvDescriptors[i].cpuHandle);

		if (dsvDescriptor.valid)
			dsv = dsvDescriptor.cpuHandle;

		commandList->OMSetRenderTargets(rtvDescriptors.size(), rtvHandles.data(), FALSE, dsvDescriptor.valid ? &dsv : nullptr);
	}

	void CommandList::SetViewport(FLOAT width, FLOAT height, FLOAT topLeftX, FLOAT topLeftY)
	{
		D3D12_VIEWPORT viewport{};
		viewport.TopLeftX = topLeftX;
		viewport.TopLeftY = topLeftY;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissor{};
		scissor.left = static_cast<LONG>(topLeftX);
		scissor.top = static_cast<LONG>(topLeftY);
		scissor.right = static_cast<LONG>(topLeftX + width);
		scissor.bottom = static_cast<LONG>(topLeftY + height);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissor);
	}

	void CommandList::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		commandList->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)topology);
	}

	void CommandList::ClearRenderTarget(const std::vector<DescriptorHeap::Descriptor>& descriptors, const DirectX::XMFLOAT4& color)
	{
		float clearColor[4] = { color.x,color.y,color.z,color.w };
		for (auto& descriptor : descriptors)
		{
			commandList->ClearRenderTargetView(descriptor.cpuHandle, clearColor, 0, nullptr);
		}
	}

	void CommandList::ClearDepthTarget(DescriptorHeap::Descriptor descriptor)
	{
		commandList->ClearDepthStencilView(descriptor.cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void CommandList::ClearUAVUint(Buffer::Ref buffer) {
		UINT clearValue[4] = { 0,0,0,0 };
		commandList->ClearUnorderedAccessViewUint(
			buffer->GetUAV().gpuHandle, buffer->GetUAV().cpuHandle,
			buffer->GetResource(), clearValue, 0, nullptr
		);
	}

	void CommandList::DrawInstanced(UINT vertexCountPerInstance, UINT nInstances)
	{
		this->
		commandList->DrawInstanced(vertexCountPerInstance, nInstances, 0, 0);
	}

	void CommandList::DrawInstancedIndexed(UINT indexCountPerInstance, UINT nInstance, UINT startIndexLocation, UINT startVertexLocation, UINT startInstanceIndex)
	{
		commandList->DrawIndexedInstanced(indexCountPerInstance, nInstance, startIndexLocation, startVertexLocation, startInstanceIndex);
	}

	void CommandList::Dispatch(int x, int y, int z)
	{
		if (type == D3D12_COMMAND_LIST_TYPE_COPY)
		{
			std::cout << "[ERROR] :: Cannot not call dispatch on a Copy Command List. Be Wise." << std::endl;
			return;
		}
		commandList->Dispatch(x, y, z);
	}

	void CommandList::ExecuteIndirect(IndirectCommandBuffer::Ref indirectCommandBuffer)
	{
		commandList->ExecuteIndirect(indirectCommandBuffer->GetCommandSignature(), indirectCommandBuffer->GetMaxCommandCount(),
			indirectCommandBuffer->GetArgumentBufferResource(), 0, indirectCommandBuffer->GetCountBufferResource(), 0);
	}

	void CommandList::CopyTextureToTexture(Texture::Ref srcTexture, Texture::Ref dstTexture)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		srcLocation.pResource = srcTexture->GetResource();
		srcLocation.SubresourceIndex = 0;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION dstLocation{};
		dstLocation.pResource = dstTexture->GetResource();
		dstLocation.SubresourceIndex = 0;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	void CommandList::CopyBufferToTexture(Buffer::Ref srcBuffer, Texture::Ref dstTexture, int subTextureIndex)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		srcLocation.pResource = srcBuffer->GetResource();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint = {
			.Format = (DXGI_FORMAT)dstTexture->GetFormat(),
			.Width = dstTexture->GetWidth(),
			.Height = dstTexture->GetHeight(),
			.Depth = 1,
			.RowPitch = dstTexture->GetWidth() * Texture::GetPixelSize(dstTexture->GetFormat()),
		};
		
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};
		dstLocation.pResource = dstTexture->GetResource();
		dstLocation.SubresourceIndex = subTextureIndex;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	void CommandList::CopyTextureToBuffer(Texture::Ref srcTexture, Buffer::Ref dstBuffer)
	{
		UINT pixelSize = Texture::GetPixelSize(srcTexture->GetFormat());
		UINT rowPitch = (srcTexture->GetWidth() * pixelSize + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
			& ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

		D3D12_TEXTURE_COPY_LOCATION copySource = {};
		copySource.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copySource.pResource = srcTexture->GetResource();
		copySource.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION copyDest = {};
		copyDest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		copyDest.pResource = dstBuffer->GetResource();
		copyDest.PlacedFootprint.Offset = 0;
		copyDest.PlacedFootprint.Footprint.Format = DXGI_FORMAT(srcTexture->GetFormat());
		copyDest.PlacedFootprint.Footprint.Width = srcTexture->GetWidth();
		copyDest.PlacedFootprint.Footprint.Height = srcTexture->GetHeight();
		copyDest.PlacedFootprint.Footprint.Depth = 1;
		copyDest.PlacedFootprint.Footprint.RowPitch = rowPitch;

		commandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySource, nullptr);
	}

	void CommandList::CopyBufferToCubeMapTexture(Buffer::Ref srcBuffer, CubeMap::Ref cubeMap, int subTextureResource)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		srcLocation.pResource = srcBuffer->GetResource();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint = {
			.Format = (DXGI_FORMAT)cubeMap->GetFormat(),
			.Width = cubeMap->GetWidth(),
			.Height = cubeMap->GetHeight(),
			.Depth = 1,
			.RowPitch = cubeMap->GetWidth() * Texture::GetPixelSize(cubeMap->GetFormat()),
		};

		D3D12_TEXTURE_COPY_LOCATION dstLocation{};
		dstLocation.pResource = cubeMap->GetResource();
		dstLocation.SubresourceIndex = subTextureResource;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	}
	void CommandList::CopyBufferToBuffer(Buffer::Ref srcBuffer, UINT srcOffset, Buffer::Ref dstBuffer, UINT64 numBytes, bool align)
	{
		UINT finalBytes;
		if (align)
			finalBytes = (numBytes) ? (numBytes + 255) & ~255 : dstBuffer->GetSize();
		else 
			finalBytes = numBytes;

		commandList->CopyBufferRegion(dstBuffer->GetResource(), 0, srcBuffer->GetResource(), srcOffset, finalBytes);
	}
}