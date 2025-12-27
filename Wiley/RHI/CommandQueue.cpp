#include "CommandQueue.h"
#include "Fence.h"
#include "Tracy/tracy/Tracy.hpp"

namespace RHI
{
	CommandQueue::CommandQueue(Device::Ref device, CommandListType _type,const std::string& name)
		:_device(device), type((D3D12_COMMAND_LIST_TYPE)_type)
	{
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		cmdQueueDesc.Type = type;

		HRESULT result = device->GetNative()->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue));
		if (FAILED(result))
			std::cout << "Failed to create command queue of type." << (int)type << std::endl;

#ifdef _DEBUG
		wchar_t lName[256];
		swprintf_s(lName, 256, L"%hs", name.c_str());
		commandQueue->SetName(lName);
#endif // _DEBUG

	}

	CommandQueue::~CommandQueue()
	{
		commandQueue.Reset();
		_device.reset();
	}

	void RHI::CommandQueue::Submit(const std::vector<CommandList::Ref>& commandLists)
	{
		ZoneScopedN("CommandQueue::Submit");

		std::vector<ID3D12CommandList*> pCmdLists(commandLists.size());
		int i = 0;
		for (auto& list : commandLists)
		{
			pCmdLists[i] = (list->GetCommandList());
			i++;
		}

		commandQueue->ExecuteCommandLists(pCmdLists.size(), pCmdLists.data());
	}

	void CommandQueue::Wait(Fence::Ref fence)
	{
		fence->Signal(this);
		fence->BlockCPU();
	}
}
