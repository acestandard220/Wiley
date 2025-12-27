#ifndef _INDIRECT_COMMAND_BUFFER_H
#define _INDIRECT_COMMAND_BUFFER_H

//Created on 11/28/2025 at 1:38

#include "Device.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"
#include "Buffer.h"

#include "../Core/defines.h"
#include "../Core/Utils.h"

#define MAX_COMMANDS 1000

namespace RHI
{
	enum class IndirectCommandType {
		DrawInstanced = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW,
		DrawIndexedInstanced = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED,
		Dispatch = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH,

		Unknown
	};

	struct IndirectCommandBufferDesc
	{
		IndirectCommandType commandType = IndirectCommandType::Unknown;
		UINT maxCommandCount = 0;
		UINT commandPerStruct = 0;
		RootSignature::Ref rootSignature = nullptr;
	};

	class IndirectCommandBuffer
	{
		public:
			struct DrawInstancedIndexedArgs
			{
				UINT constant = 0;
				//UINT __padding[3];
				

				UINT indexCountPerInstance = 0;
				UINT instanceCount = 0;
				UINT indexStartLocation = 0;
				UINT vertexStartLocation = 0;
				UINT instanceStartLocation = 0;
			};

			struct DrawInstancedArgs
			{
				UINT vertexCountPerInstance = 0;
				UINT instanceCount = 0;
				UINT vertexStartLocation = 0;
				UINT instanceStartLocation = 0;
			};

			struct DispatchArgs {
				UINT threadGroupCountX = 0;
				UINT threadGroupCountY = 0;
				UINT threadGroupCountZ = 0;
			};

		public:
			using Ref = std::shared_ptr<IndirectCommandBuffer>;
			IndirectCommandBuffer(Device::Ref device, IndirectCommandType commandType, UINT maxCmdCount, UINT cmdPerStruct,
				RootSignature::Ref rootSignature, DescriptorHeap::Heap heaps, const std::string& name);
			~IndirectCommandBuffer();

			WILEY_NODISCARD static IndirectCommandBuffer::Ref CreateIndirectCommandBuffer(Device::Ref device, 
				IndirectCommandBufferDesc indirectCommandBufferDesc, DescriptorHeap::Heap heaps, const std::string& name);

			DescriptorHeap::Descriptor GetCommandUAV()const { return commandBufferUav; }
			DescriptorHeap::Descriptor GetCounterUAV()const { return counterBufferUav; }

			IndirectCommandType GetCommandType()const { return commandType; }
			UINT GetMaxCommandCount()const { return maxCommandCount; }

			Buffer::Ref GetArgumentBuffer()const { return _argumentBuffer; }
			Buffer::Ref GetCountBuffer()const { return _countBuffer; }

			ID3D12CommandSignature* GetCommandSignature()const { return commandSignature.Get(); }
			ID3D12Resource* GetArgumentBufferResource()const { return _argumentBuffer->GetResource(); }
			ID3D12Resource* GetCountBufferResource()const { return _countBuffer->GetResource(); }

		private:
			void CreateBufferResources();
		private:
			IndirectCommandType commandType = IndirectCommandType::Unknown;
			const UINT maxCommandCount;
			const UINT cmdPerStruct;

			DescriptorHeap::Descriptor commandBufferUav;
			DescriptorHeap::Descriptor counterBufferUav;

			Device::Ref _device;
			
			Buffer::Ref _argumentBuffer;
			Buffer::Ref _countBuffer;

			ComPtr<ID3D12CommandSignature> commandSignature;
	};

}


#endif // !_INDIRECT_COMMAND_BUFFER_H
