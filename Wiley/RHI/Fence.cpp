#include "Fence.h"

namespace RHI
{
	RHI::Fence::Fence(Device::Ref device,const std::string& name)
		:_device(device), fenceValue(0), fenceWaitValue(0), done(true)
	{
		HRESULT result = device->GetNative()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		if (FAILED(result))
		{
			std::cout << "Failed to create fence object.";
		}

		WILEY_NAME_D3D12_OBJECT(fence, name);
	}

	Fence::~Fence()
	{
		fence.Reset();
		_device.reset();
	}

	void Fence::Signal(CommandQueue* commandQueue)
	{
		fenceWaitValue = ++fenceValue;
		commandQueue->GetQueue()->Signal(fence.Get(), fenceWaitValue);
		done = false;
	}

	void Fence::BlockCPU()
	{
		if (fence->GetCompletedValue() < fenceWaitValue)
		{
			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			ThrowIfFailed(fence->SetEventOnCompletion(fenceWaitValue, fenceEvent));
			DWORD result = WaitForSingleObject(fenceEvent, INFINITE);
			if (result == WAIT_TIMEOUT)
			{
				std::cout << "GPU time-out. Stall detected." << std::endl;
				exit(-4);
			}
			CloseHandle(fenceEvent);
		}
		done = true;
	}

	void Fence::BlockGPU(CommandQueue* commandQueue)
	{
		commandQueue->GetQueue()->Wait(fence.Get(), fenceWaitValue);
	}

	void Fence::GPUWaitForValue(CommandQueue* commandQueue, UINT waitValue)
	{
		commandQueue->GetQueue()->Wait(fence.Get(), waitValue);
	}
}