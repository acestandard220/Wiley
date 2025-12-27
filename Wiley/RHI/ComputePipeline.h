#pragma once

//Created on 11/22/2026 at 11:16
//First time with compute shaders

#include "Device.h"
#include "RootSignature.h"

namespace RHI
{
	struct ComputePipelineSpecs {
		ShaderByteCode computeByteCode;
		RootSignatureSpecs rootSpecs;
	};

	class ComputePipeline
	{
	public:
		using Ref = std::shared_ptr<ComputePipeline>;

		ComputePipeline(Device::Ref device, ComputePipelineSpecs specs, const std::string name);
		~ComputePipeline();

		static ComputePipeline::Ref CreateComputePipeline(Device::Ref device, ComputePipelineSpecs specs, const std::string name = "ComputePipeline");

		ID3D12PipelineState* GetNative() { return computePipelineState; }
		RootSignature::Ref GetRootSignature() { return rootSignature; }

	private:
		Device::Ref _device;

		ID3D12PipelineState* computePipelineState;
		RootSignature::Ref rootSignature;
	};
 
}
