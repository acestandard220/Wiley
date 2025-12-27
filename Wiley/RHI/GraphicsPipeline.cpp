#include "GraphicsPipeline.h"

namespace RHI
{
	GraphicsPipeline::GraphicsPipeline(Device::Ref device, const GraphicsPipelineSpecs& specs, const std::string& name)
		:_device(device)
	{
		D3D12_SHADER_DESC vertexShaderDesc{};
		auto it = specs.byteCodes.find(ShaderType::Vertex);
		if (it == specs.byteCodes.end())
			std::cout << "Cannot create Graphics Pipeline without vertex shader.\n";

		ID3D12ShaderReflection* vertexShaderReflection = ShaderCompiler::GetReflection(it->second, vertexShaderDesc);
		
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements(vertexShaderDesc.InputParameters);

		for (int i = 0; i < vertexShaderDesc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC parameterSignature{};

			vertexShaderReflection->GetInputParameterDesc(i, &parameterSignature);

			D3D12_INPUT_ELEMENT_DESC inputElement{};
			inputElement.SemanticName = parameterSignature.SemanticName;
			inputElement.SemanticIndex = parameterSignature.SemanticIndex;
			inputElement.InputSlot = 0;
			inputElement.InstanceDataStepRate = 0;
			inputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			inputElement.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

			if (parameterSignature.Mask == 1)
			{
				if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32_UINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32_SINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (parameterSignature.Mask <= 3)
			{
				if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32G32_UINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32G32_SINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (parameterSignature.Mask <= 7)
			{
				if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (parameterSignature.Mask <= 15)
			{
				if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (parameterSignature.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			inputElements[i] = inputElement;
		}


		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.NodeMask = 0;

		desc.InputLayout.NumElements = static_cast<uint32_t>(inputElements.size());
		desc.InputLayout.pInputElementDescs = inputElements.data();

		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		rootSignature = std::make_shared<RootSignature>(device, specs.rootSignatureSpecs);
		desc.pRootSignature = rootSignature->GetNative();

		if (specs.depth)
		{
			desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

			desc.DepthStencilState.DepthEnable = TRUE;
			desc.DepthStencilState.DepthWriteMask = (specs.depthWrite) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			desc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)specs.depthFunc;

			desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		}
		else
		{
			desc.DepthStencilState.DepthEnable = FALSE;
			desc.DepthStencilState.StencilEnable = FALSE;
			desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		}

		
		desc.NumRenderTargets = specs.nRenderTarget;
		for (int i = 0; i < specs.nRenderTarget; i++)
			desc.RTVFormats[i] = (DXGI_FORMAT)specs.textureFormats[i];

		desc.PrimitiveTopologyType = specs.line ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);


		//Problem dey better call Jesus
		
		desc.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(specs.cullMode);
		desc.RasterizerState.FillMode = static_cast<D3D12_FILL_MODE>(specs.fillMode);
		desc.RasterizerState.FrontCounterClockwise = !static_cast<bool>(specs.frontFace);

		desc.SampleDesc.Count = 1;
		desc.SampleMask = UINT_MAX;

		it = specs.byteCodes.find(ShaderType::Vertex);
		if (it != specs.byteCodes.end())
		{
			desc.VS.pShaderBytecode = (it->second.byteCode->GetBufferPointer());
			desc.VS.BytecodeLength = it->second.byteCode->GetBufferSize();
		}
		it = specs.byteCodes.find(ShaderType::Pixel);
		if (it != specs.byteCodes.end())
		{
			desc.PS.pShaderBytecode = (it->second.byteCode->GetBufferPointer());
			desc.PS.BytecodeLength = it->second.byteCode->GetBufferSize();
		}


		HRESULT result = device->GetNative()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
		if (FAILED(result))
		{
			std::cout << "Failed to create pipeline state.\n";
		}

#ifdef _DEBUG

		wchar_t lName[256];
		swprintf_s(lName, 256, L"%hs", name.c_str());
		pipelineState->SetName(lName);

#endif // _DEBUG

		vertexShaderReflection->Release();

	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		pipelineState.Reset();
		_device.reset();
	}

}