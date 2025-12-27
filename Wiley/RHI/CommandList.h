#pragma once
#include "Device.h"
#include "DescriptorHeap.h"
#include "RootSignature.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "Texture.h"
#include "Buffer.h"
#include "IndirectCommandBuffer.h"
#include "CubeMap.h"

namespace RHI
{
	enum class CommandListType;

	enum class PrimitiveTopology
	{
		TriangleList = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		LineList = D3D_PRIMITIVE_TOPOLOGY_LINELIST
	};

	struct Barrier
	{
		Texture::Ref texture = nullptr;
		TextureUsage newState = TextureUsage::Common;
	};

	class CommandList
	{
	public:
		using Ref = std::shared_ptr<CommandList>;

		CommandList(Device::Ref device, CommandListType commandListType,const std::string& name = "Command_Buffer404");
		~CommandList();

		static Ref CreateCommandList(Device::Ref device, CommandListType commandListType, const std::string& name = "Command_Buffer404");

		ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }
		ID3D12CommandAllocator* GetCommandAllocator() { return commandAllocator.Get(); }

		void Continue(const std::vector<DescriptorHeap::Ref>& heap = {});
		void Begin(const std::vector<DescriptorHeap::Ref>& heap = {});
		void End();

		void ImageBarrier(Texture::Ref texture, TextureUsage newState);
		void ImageBarrier(std::vector<Barrier> barriers);

		//Explicit
		void BufferBarrier(Buffer::Ref buffer, D3D12_RESOURCE_STATES usage);
		void CubeMapBarrier(CubeMap::Ref, D3D12_RESOURCE_STATES usage);

		void BufferUAVToCopySource(Buffer::Ref buffer);
		void BufferCopySourceToUAV(Buffer::Ref buffer);

		void BufferUAVToCopyDest(Buffer::Ref buffer);
		void BufferCopyDestToUAV(Buffer::Ref buffer);

		void BufferUAVToNonPixelShader(const std::vector<Buffer::Ref>& buffer);
		void BufferNonPixelShaderToUAV(const std::vector<Buffer::Ref>& buffer);

		void BufferUAVToPixelShader(const std::vector<Buffer::Ref>& buffer);
		void BufferPixelShaderToUAV(const std::vector<Buffer::Ref>& buffer);

		void BufferUAVToAllShader(Buffer::Ref buffer);
		void BufferAllShaderToUAV(Buffer::Ref buffer);

		void BufferUAVToCommon(const std::vector<Buffer*>& buffer);
		void BufferCommonToUAV(const std::vector<Buffer*>& buffer);

		void UAVBarrier(Buffer::Ref buffer);

		//Debug purposes
		void ImageBarrierSingles(std::vector<Barrier> barriers);

		void SetGraphicsPipeline(GraphicsPipeline::Ref graphicsPipeline);
		void SetComputePipeline(ComputePipeline::Ref computePipeline);
		void SetGraphicsRootSignature(RootSignature::Ref rootSignature);
		void SetComputeRootSignature(RootSignature::Ref rootSignature);

		/// <summary>
		///		Set shader resources described by the root signature
		/// </summary>
		/// <param name="descriptor"></param>
		/// <param name="index">
		/// The index param represents tha root parameter index. 
		/// This is not the desired register number provided.
		/// The root parameter index is automatically defined by the order in which shader resources are defined for a shader
		/// </param>
		void BindShaderResource(DescriptorHeap::Descriptor descriptor, int index);
		void BindSamplerResource(DescriptorHeap::Descriptor descriptor, int index);
		void PushConstant(const void* data, size_t size, uint32_t index);

		void BindComputeShaderResource(DescriptorHeap::Descriptor descriptor,int index);
		void PushComputeConstant(const void* data, size_t size, uint32_t index);
		void BindComputeSamplerResource(DescriptorHeap::Descriptor descriptor, int index);

		void BindConstantBuffer(DescriptorHeap::Descriptor descriptor, int index);
		void BindVertexBuffer(Buffer::Ref buffer);

		void BindIndexBuffer(Buffer::Ref buffer);

		void SetDescriptors(const std::vector<DescriptorHeap::Ref>& heaps);

		void SetRenderTargets(const std::vector<DescriptorHeap::Descriptor>& rtvDescriptors, DescriptorHeap::Descriptor dsvDescriptor);

		void SetViewport(FLOAT width, FLOAT height, FLOAT topLeftX, FLOAT topLeftY);
		void SetPrimitiveTopology(PrimitiveTopology topology);

		void ClearRenderTarget(const std::vector<DescriptorHeap::Descriptor>& descriptors, const DirectX::XMFLOAT4& color);
		void ClearDepthTarget(DescriptorHeap::Descriptor descriptor);
		void ClearUAVUint(Buffer::Ref buffer);

		void DrawInstanced(UINT vertexCountPerInstance,UINT nInstances);
		void DrawInstancedIndexed(UINT indexCountPerInstance, UINT nInstance, UINT startIndexLocation = 0, UINT startVertexLocation = 0, UINT startInstanceIndex = 0);
		void Dispatch(int x, int y, int z);
		void ExecuteIndirect(IndirectCommandBuffer::Ref indirectCommandBuffer);

		void CopyTextureToTexture(Texture::Ref srcTexture, Texture::Ref dsTexture);
		void CopyBufferToTexture(Buffer::Ref srcBuffer, Texture::Ref dstTexture, int subTextureIndex = 0);
		void CopyTextureToBuffer(Texture::Ref srcBuffer, Buffer::Ref dstTexture);

		void CopyBufferToCubeMapTexture(Buffer::Ref srcBuffer, CubeMap::Ref cubeMap, int sub);

		/// <summary>
		/// Copies a specified number of bytes from a source buffer (at a given offset) into a destination buffer.
		/// </summary>
		/// <param name="srcBuffer">Reference to the source buffer to read from.</param>
		/// <param name="srcOffset">Byte offset within srcBuffer where copying begins (UINT).</param>
		/// <param name="dstBuffer">Reference to the destination buffer to write to.</param>
		/// <param name="numBytes">Number of bytes to copy (UINT64). numBytes = 0 uses the copies the entire size of the dst buffer</param>
		void CopyBufferToBuffer(Buffer::Ref srcBuffer, UINT srcOffset, Buffer::Ref dstBuffer, UINT64 numBytes, bool align = true);
		
	private:
		ComPtr<ID3D12GraphicsCommandList> commandList;
		ComPtr<ID3D12CommandAllocator> commandAllocator;

		Device::Ref _device;
		D3D12_COMMAND_LIST_TYPE type;
	};
}

