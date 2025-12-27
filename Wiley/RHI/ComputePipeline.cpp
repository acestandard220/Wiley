#include "ComputePipeline.h"

namespace RHI
{
	ComputePipeline::ComputePipeline(Device::Ref device, ComputePipelineSpecs specs, const std::string name) {
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.CS.pShaderBytecode = specs.computeByteCode.byteCode->GetBufferPointer();
		desc.CS.BytecodeLength = specs.computeByteCode.byteCode->GetBufferSize();

		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		rootSignature = std::make_shared<RootSignature>(device, specs.rootSpecs, name + "_RootSignature");
		desc.pRootSignature = rootSignature->GetNative();

		HRESULT result = device->GetNative()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&computePipelineState));
		if (FAILED(result))
		{
			std::cout << "Failed to create compute pipelinestate :: " << name << std::endl;
		}
	}

	ComputePipeline::~ComputePipeline()
	{
		rootSignature.reset();
		_device.reset();
	}

	ComputePipeline::Ref ComputePipeline::CreateComputePipeline(Device::Ref device, ComputePipelineSpecs specs, const std::string name)
	{
		return std::make_shared<ComputePipeline>(device, specs, name);
	}

}
