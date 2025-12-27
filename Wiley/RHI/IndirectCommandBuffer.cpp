#include "IndirectCommandBuffer.h"

namespace RHI {

	UINT GetCommandTypeStride(IndirectCommandType type)
	{
		switch (type) {
			case IndirectCommandType::DrawInstanced: return sizeof(IndirectCommandBuffer::DrawInstancedArgs);
			case IndirectCommandType::DrawIndexedInstanced: return sizeof(IndirectCommandBuffer::DrawInstancedIndexedArgs);
			case IndirectCommandType::Dispatch: return sizeof(IndirectCommandBuffer::DispatchArgs);
			default:return 0;
		}
	}

	RHI::IndirectCommandBuffer::IndirectCommandBuffer(Device::Ref device, IndirectCommandType commandType, UINT maxCmdCount, UINT cmdPerStruct, RootSignature::Ref rootSignature, DescriptorHeap::Heap heaps, const std::string& name)
		:_device(device), commandType(commandType), maxCommandCount(maxCmdCount), cmdPerStruct(cmdPerStruct)
	{

		D3D12_INDIRECT_ARGUMENT_DESC indirectArgDesc[2];
		indirectArgDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		indirectArgDesc[0].Constant.DestOffsetIn32BitValues = 0;
		indirectArgDesc[0].Constant.Num32BitValuesToSet = 1;
		indirectArgDesc[0].Constant.RootParameterIndex = 0;

		indirectArgDesc[1].Type = (D3D12_INDIRECT_ARGUMENT_TYPE)commandType;
 
		D3D12_COMMAND_SIGNATURE_DESC signatureDesc{};
		signatureDesc.ByteStride = GetCommandTypeStride(commandType);
		signatureDesc.NumArgumentDescs = cmdPerStruct;
		signatureDesc.pArgumentDescs = indirectArgDesc;
		signatureDesc.NodeMask = 0;

		HRESULT result = device->GetNative()->CreateCommandSignature(&signatureDesc, (rootSignature) ? rootSignature->GetNative() : nullptr,
			IID_PPV_ARGS(&commandSignature));
		if (FAILED(result))
		{
			std::cout << "Failed to create command signature." << std::endl;
		}
		
		CreateBufferResources();

		//Build Descriptors
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC commandUavDesc{};
			commandUavDesc.Format = DXGI_FORMAT_UNKNOWN;
			commandUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			commandUavDesc.Buffer = {
				.FirstElement = 0,
				.NumElements = maxCmdCount,
				.StructureByteStride = signatureDesc.ByteStride,
				.CounterOffsetInBytes = 0,
				.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
			};

			commandBufferUav = heaps.cbv_srv_uav->Allocate();
			if (!commandBufferUav.valid) {
				std::cout << "Failed to create Unordered Access View for Indirect Command Buffer." << std::endl;
			}
			device->GetNative()->CreateUnorderedAccessView(_argumentBuffer->GetResource(), _countBuffer->GetResource(),
				&commandUavDesc, commandBufferUav.cpuHandle);


			D3D12_UNORDERED_ACCESS_VIEW_DESC counterUavDesc{};
			counterUavDesc.Format = DXGI_FORMAT_UNKNOWN;
			counterUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			counterUavDesc.Buffer = {
				.FirstElement = 0,
				.NumElements = 1,
				.StructureByteStride = 4,
				.CounterOffsetInBytes = 0,
				.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
			};

			counterBufferUav = heaps.cbv_srv_uav->Allocate();
			if (!counterBufferUav.valid) {
				std::cout << "Failed to create Unoredered Access View for Indirect Counter Buffer." << std::endl;
			}
			device->GetNative()->CreateUnorderedAccessView(_countBuffer->GetResource(), nullptr, &counterUavDesc,
				counterBufferUav.cpuHandle);
		}

		std::string resourceName = name + "_ArgumentBufferResource";

		_argumentBuffer->GetResource()->SetName(L"Argument Buffer");
		_countBuffer->GetResource()->SetName(L"Count Buffer");

		WILEY_NAME_D3D12_OBJECT(commandSignature, name);
	}

	IndirectCommandBuffer::~IndirectCommandBuffer()
	{
		//_argumentBuffer.Reset();
		commandSignature.Reset();

		_device.reset();
	}

	IndirectCommandBuffer::Ref IndirectCommandBuffer::CreateIndirectCommandBuffer(Device::Ref device, IndirectCommandBufferDesc indirectCommandBufferDesc, DescriptorHeap::Heap heaps, const std::string& name)
	{
		return std::make_shared<IndirectCommandBuffer>(device, indirectCommandBufferDesc.commandType,
			indirectCommandBufferDesc.maxCommandCount, indirectCommandBufferDesc.commandPerStruct,
			indirectCommandBufferDesc.rootSignature, heaps, name);
	}
	
	void IndirectCommandBuffer::CreateBufferResources() {

		_argumentBuffer = std::make_shared<Buffer>(BufferUsage::ComputeStorage, sizeof(DrawInstancedIndexedArgs) * maxCommandCount);
		_countBuffer = std::make_shared<Buffer>(BufferUsage::ComputeStorage, 4);

		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(DrawInstancedIndexedArgs) * maxCommandCount, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		HRESULT result = _device->GetNative()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(_argumentBuffer->GetResourceAddress()));
		if (FAILED(result))
			std::cout << "Failed to create Argument Buffer Resource." << std::endl;

		CD3DX12_HEAP_PROPERTIES countBufferHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC countBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		result = _device->GetNative()->CreateCommittedResource(&countBufferHeapProperties, D3D12_HEAP_FLAG_NONE, &countBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(_countBuffer->GetResourceAddress()));
		if (FAILED(result))
			std::cout << "Failed to create Count Buffer Resource." << std::endl;
	}
}