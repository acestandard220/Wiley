#pragma once
#include <unordered_map>

#include "Device.h"
#include "Texture.h"
#include "ShaderByteCode.h"
#include "RootSignature.h"


namespace RHI
{
	enum class FillMode
	{
		WireFrame = D3D12_FILL_MODE_WIREFRAME,
		Solid = D3D12_FILL_MODE_SOLID
	};

	enum class CullMode
	{
		Back = D3D12_CULL_MODE_BACK,
		Front = D3D12_CULL_MODE_FRONT,
		None = D3D12_CULL_MODE_NONE
	};

	enum class FrontFace
	{
		ClockWise = 0,
		CounterClockWise = 1
	};

	enum class DepthFunc {
		Less = D3D12_COMPARISON_FUNC_LESS,
		LessEqual = D3D12_COMPARISON_FUNC_LESS_EQUAL,
		Equal = D3D12_COMPARISON_FUNC_EQUAL
	};

	struct GraphicsPipelineSpecs
	{
		bool line = false;

		bool depth = false;
		bool depthWrite = true;
		DepthFunc depthFunc = DepthFunc::Less;
		TextureFormat depthStencilFormat;

		int nRenderTarget;
		TextureFormat textureFormats[8];
		
		FillMode fillMode = FillMode::Solid;
		CullMode cullMode = CullMode::None;
		FrontFace frontFace = FrontFace::CounterClockWise;

		std::unordered_map<RHI::ShaderType, ShaderByteCode> byteCodes;
		RootSignatureSpecs rootSignatureSpecs;
	};

	class GraphicsPipeline
	{
	public:
		using Ref = std::shared_ptr<GraphicsPipeline>;

		GraphicsPipeline(Device::Ref device,const GraphicsPipelineSpecs& specs, const std::string& name = "Graphics_Pipeline");
		~GraphicsPipeline();

		RootSignature::Ref GetRootSignature() { return rootSignature; }
		ID3D12PipelineState* GetNative() { return pipelineState.Get(); }

	private:
		Device::Ref _device;

		RootSignature::Ref rootSignature;
		ComPtr<ID3D12PipelineState> pipelineState;
	};
}

