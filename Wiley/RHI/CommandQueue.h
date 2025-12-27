#pragma once
#include "Device.h"
#include "CommandList.h"


namespace RHI
{
	enum class CommandListType
	{
		CommandListDirect = D3D12_COMMAND_LIST_TYPE_DIRECT,
		CommandListCopy = D3D12_COMMAND_LIST_TYPE_COPY,
		CommandListCompute = D3D12_COMMAND_LIST_TYPE_COMPUTE
	};

	class Fence;
	class CommandQueue
	{
		public:
			using Ref = std::shared_ptr<CommandQueue>;

			CommandQueue(Device::Ref device, CommandListType type,const std::string& name = "Command_Queue");
			~CommandQueue();

			static Ref CreateCommandQueue(Device::Ref device, CommandListType type, const std::string& name =  "Command_Queue") { return std::make_shared<CommandQueue>(device, type,name); }

			ID3D12CommandQueue* GetQueue() { return commandQueue.Get(); }

			void Wait(std::shared_ptr<Fence> fence);

			void Submit(const std::vector<CommandList::Ref>& commandLists);

		private:
			Device::Ref _device;
			ComPtr<ID3D12CommandQueue> commandQueue;
			D3D12_COMMAND_LIST_TYPE type;
	};

}
