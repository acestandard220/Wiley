#pragma once
#include "../Core/Utils.h"
#include "Device.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "CommandList.h"
#include "Buffer.h"
#include "Texture.h"
#include "GraphicsPipeline.h"
#include "RootSignature.h"
#include "Fence.h"
#include "Sampler.h"
#include "ComputePipeline.h"
#include "IndirectCommandBuffer.h"
#include "CubeMap.h"

#include "../Core/Window.h"

#include "Tracy/tracy/Tracy.hpp"


namespace RHI
{
	//This class represents a factory/abstraction for communicating with the GPU.
	//Through this class you can query GPU data, create resource etc.
	//This class is not responsible for holding any game/scene states. All states held in this class are GPU related.
	class RenderContext
	{
	public:
		using Ref = std::shared_ptr<RenderContext>;

		RenderContext(Wiley::Window::Ref window);
		~RenderContext();

		void Resize(uint32_t width, uint32_t height);

		void Present(bool vsync = false);

		void WaitForGPU();
		void NewFrame();

		WILEY_NODISCARD IndirectCommandBuffer::Ref CreateIndirectCommandBuffer(IndirectCommandBufferDesc indirectCommandBufferDesc, const std::string& name = "IndirectCommandBuffer");

		//Dynamic Resource Work
		WILEY_NODISCARD Texture::Ref CreateShaderResourceTexture(void* data, int width, int height, int nChannel, int bitPerChannel);
		WILEY_NODISCARD Texture::Ref CreateShaderResourceTextureFromFile(const std::string& name, int width, int height, int nChannel, int bitPerChannel);
		WILEY_NODISCARD std::vector<uint8_t> GetTextureBytes(RHI::Texture::Ref texture);
		WILEY_NODISCARD CubeMap::Ref CreateShaderResourceCubeMap(float* data, uint32_t width, uint32_t height, int nChannel, int bitPerChannel, const std::string& name);
		WILEY_NODISCARD CubeMap::Ref ConvoluteCubeMap(CubeMap::Ref cubmap, uint32_t convSize, const std::string& name);

		WILEY_NODISCARD Texture::Ref CreateTexture(TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage, const std::string& name = "Texture");

		WILEY_NODISCARD Buffer::Ref CreateUploadBuffer(uint64_t size, uint64_t stride,const std::string& name = "UploadBuffer");
		WILEY_NODISCARD Buffer::Ref CreateComputeStorageBuffer(uint64_t size, uint64_t stride, const std::string& name = "ComputeStorageBuffer");

		WILEY_NODISCARD Buffer::Ref CreateVertexBuffer(uint64_t size, uint64_t stride, const std::string& name = "VertexBuffer");
		WILEY_NODISCARD Buffer::Ref CreateIndexBuffer(uint64_t size, uint64_t stride, const std::string& name = "IndexBuffer");
		WILEY_NODISCARD Buffer::Ref CreateConstantBuffer(bool persistent, uint64_t size, const std::string& name = "ConstantBuffer");

		WILEY_NODISCARD GraphicsPipeline::Ref CreateGraphicsPipeline(GraphicsPipelineSpecs specs);
		WILEY_NODISCARD RootSignature::Ref CreateRootSignature(RootSignatureSpecs specs, const std::string& name = "Root_Signature");
		WILEY_NODISCARD Sampler::Ref CreateSampler(SamplerAddress address, SamplerFilter filter);

		WILEY_NODISCARD CubeMap::Ref CreateCubeMap(uint32_t width, uint32_t height,TextureFormat formate,const std::string& name = "CubeMap");

		void BindGraphicsPipeline(GraphicsPipeline::Ref graphicsPipeline);

		void ExecuteGraphicsCommandList(const std::vector<CommandList::Ref> list);
		void ExecuteCopyCommandList(const std::vector<CommandList::Ref> list);
		void ExecuteComputeCommandList(const std::vector <CommandList::Ref>list);

		void TransitionSwapchain();

		DescriptorHeap::Heap& GetDescriptorHeaps() { return heaps; }

		DescriptorHeap::Descriptor AllocateCBV_SRV_UAV() const;
		std::vector<DescriptorHeap::Descriptor> AllocateCBV_SRV_UAV(UINT nDescriptors) const;
		DescriptorHeap::Descriptor AllocateSampler() const;
		DescriptorHeap::Descriptor AllocateRTV() const;
		DescriptorHeap::Descriptor AllocateDSV() const;

		//For Readability.
		DescriptorHeap::Descriptor AllocateSRV() const;
		DescriptorHeap::Descriptor AllocateUAV() const;
		DescriptorHeap::Descriptor AllocateCBV() const;

		uint32_t GetBackBufferIndex() { return frameIndex; }
		Texture::Ref GetBackBufferTexture() { return swapChain->GetImage(frameIndex); }
		DescriptorHeap::Descriptor GetBackBufferDescriptor() { return swapChain->GetDescriptor(frameIndex); }

		WILEY_NODISCARD Fence::Ref GetCurrentGraphicsFence() { return gfxFence[frameIndex]; }
		WILEY_NODISCARD Fence::Ref GetGraphicsFence(uint32_t index) { return gfxFence[index]; }
		WILEY_NODISCARD Fence::Ref GetCopyFence(uint32_t index) { return copyFence[index]; }
		WILEY_NODISCARD Fence::Ref GetComputeFence() { return computeFence; }

		WILEY_NODISCARD CommandQueue::Ref GetCopyCommandQueue() { return copyCommandQueue; }
		WILEY_NODISCARD CommandQueue::Ref GetCommandQueue() { return gfxCommandQueue; }
		WILEY_NODISCARD CommandQueue::Ref GetComputeQueue() { return computeCommandQueue; }

		WILEY_NODISCARD CommandList::Ref GetCurrentCommandList() { return gfxCommandList[frameIndex]; }
		WILEY_NODISCARD CommandList::Ref GetCopyCurrentCommandList() { return copyCommandList; }
		WILEY_NODISCARD CommandList::Ref GetComputeCommandList() { return computeCommandList; }

		WILEY_NODISCARD Wiley::Window::Ref GetWindow() { return window; }
		WILEY_NODISCARD Device::Ref GetDevice() { return device; }

		int frameIndex;
	private:

		Wiley::Window::Ref window;

		Device::Ref device;
		SwapChain::Ref swapChain;

		CommandQueue::Ref gfxCommandQueue;
		CommandQueue::Ref copyCommandQueue;
		CommandQueue::Ref computeCommandQueue;

		CommandList::Ref gfxCommandList[FRAMES_IN_FLIGHT];
		CommandList::Ref copyCommandList;
		CommandList::Ref computeCommandList;

		/// <summary>
		///		This command buffer is used in dynamic resource creation at runtime.
		///		This command buffer is used internally by the render context's dynamic resource creation functions.
		/// </summary>
		CommandList::Ref dynamicResourceCommandList; 

		Fence::Ref gfxFence[FRAMES_IN_FLIGHT];		
		Fence::Ref copyFence[FRAMES_IN_FLIGHT];
		Fence::Ref computeFence;
		Fence::Ref dynamicResourceFence;
		
		DescriptorHeap::Heap heaps;

		ComputePipeline::Ref environmentMapCreationPSO;
		ComputePipeline::Ref cubeMapConvolutePSO;
		Sampler::Ref linearSampler;

	};
}

