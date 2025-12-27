#pragma once
#include "Device.h"
#include "ShaderByteCode.h"

namespace RHI
{
	enum class RootSignatureEntryType
	{
		CBVRange = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		SRVRange = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		UAVRange = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		SamplerRange = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,

		Constant = 99
	};

	enum class ShaderVisibility {
		All = D3D12_SHADER_VISIBILITY_ALL,
		Vertex = D3D12_SHADER_VISIBILITY_VERTEX,
		Pixel = D3D12_SHADER_VISIBILITY_PIXEL,
		Compute = All
	};

	struct RootSignatureEntry {
		RootSignatureEntryType type;
		UINT rootParameterIndex;

		//Number of Descriptors/Number of Root Constants.
		UINT nValues = 1;
		UINT registerSpace = 0;
		ShaderVisibility shaderVisibility = ShaderVisibility::All;
	};

	struct RootSignatureSpecs
	{
		std::vector<RootSignatureEntry> entries;
	};

	class RootSignature
	{
	public:
		using Ref = std::shared_ptr<RootSignature>;
		RootSignature(Device::Ref device, const RootSignatureSpecs& signatureSpecs,const std::string& name = "Root_Signature");

		~RootSignature();

		//Not Complete
		void ReflectFromGraphicsShader() {};

		ID3D12RootSignature* GetNative() { return rootSignature.Get(); }

	private:
		Device::Ref _device;
		ComPtr<ID3D12RootSignature> rootSignature;
	};
}

