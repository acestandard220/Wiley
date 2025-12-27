#pragma once
//Author Ammishaddai Boakye Yiadom
//Created on 4/11/2026 at 1:29am

#include "Device.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"

#define FRAMES_IN_FLIGHT  3

namespace RHI
{
	class SwapChain
	{
	public:
		using Ref = std::shared_ptr<SwapChain>;

		SwapChain(Device::Ref device,CommandQueue::Ref commandQueue,HWND hwnd);
		~SwapChain();

		static Ref CreateSwapChain(Device::Ref device, CommandQueue::Ref commandQueue, HWND hwnd) { return std::make_shared<SwapChain>(device, commandQueue, hwnd); }
		void Transition(CommandList::Ref commandList);

		DescriptorHeap::Descriptor GetDescriptor(uint32_t index) { return descriptors[index]; }
		ID3D12Resource* GetBuffer(uint32_t index) { return backBuffer[index]->GetResource(); }

		Texture::Ref GetImage(uint32_t index) { return backBuffer[index]; }

		void Present(bool vsync = false);

		void Resize(uint32_t width, uint32_t height);

		UINT AcquireImage() { 
			frameIndex = swapChain->GetCurrentBackBufferIndex();
			return frameIndex; 
		}


		IDXGISwapChain3* GetSwapChain() { return swapChain.Get(); }
		UINT GetFrameIndex() { return frameIndex; }

		WILEY_NODISCARD UINT GetSamplerCount()const { return sampleCount; }
		WILEY_NODISCARD UINT GetQualityLevel()const { return qualityLevels; }

	private:

		ComPtr<IDXGISwapChain3> swapChain;

		Device::Ref _device;
		DescriptorHeap::Ref rtvHeap;
		DescriptorHeap::Descriptor descriptors[FRAMES_IN_FLIGHT];

		UINT sampleCount = 4; 
		UINT qualityLevels = 0;
		Texture::Ref backBuffer[FRAMES_IN_FLIGHT];

		HWND hwnd;

		UINT width;
		UINT height;
		UINT frameIndex;
	};

}
