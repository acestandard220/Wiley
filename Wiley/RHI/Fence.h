#pragma once
#include "Device.h"
#include "CommandQueue.h"

namespace RHI
{
	class Fence
	{
	public:
		using Ref = std::shared_ptr<Fence>;

		Fence(Device::Ref device, const std::string& name);
		~Fence();

		void Signal(CommandQueue* commandQueue);

		[[nodiscard]] static Fence::Ref CreateFence(Device::Ref device, const std::string name = "Fence")
		{
			return std::make_shared<Fence>(device, name);
		}

		ID3D12Fence* GetFence() { return fence.Get(); }
		UINT GetWaitValue()const { return fenceWaitValue; }

		void BlockCPU();
		void BlockGPU(CommandQueue* commandQueue);

		void GPUWaitForValue(CommandQueue* commandQueue, UINT waitValue);

		bool IsDone()const { return done; }

	private:
		Device::Ref _device;
		ComPtr<ID3D12Fence> fence;

		UINT fenceValue;
		UINT fenceWaitValue;

		bool done;
	};
}
