#include "RootSignature.h"

namespace RHI
{
	RootSignature::RootSignature(Device::Ref device, const RootSignatureSpecs& signatureSpecs, const std::string& name)
		:_device(device)
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> rootRanges(signatureSpecs.entries.size());
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(signatureSpecs.entries.size());

		for (int i = 0; i < signatureSpecs.entries.size(); i++)
		{
			const RootSignatureEntryType& entry = signatureSpecs.entries[i].type;
			UINT desiredIndex = signatureSpecs.entries[i].rootParameterIndex;
			UINT nValues = signatureSpecs.entries[i].nValues;
			UINT registerSpace = signatureSpecs.entries[i].registerSpace;
			D3D12_SHADER_VISIBILITY shaderVisibily = static_cast<D3D12_SHADER_VISIBILITY>(signatureSpecs.entries[i].shaderVisibility);

			if (entry == RootSignatureEntryType::Constant)
			{
				CD3DX12_ROOT_PARAMETER1& rootParam = rootParameters[i];
				rootParam.InitAsConstants(nValues, desiredIndex, registerSpace, shaderVisibily);
			}else{
				CD3DX12_DESCRIPTOR_RANGE1& rootRange = rootRanges[i];
				rootRange.Init((D3D12_DESCRIPTOR_RANGE_TYPE)entry, nValues, desiredIndex, registerSpace);

				CD3DX12_ROOT_PARAMETER1& rootParam = rootParameters[i];
				rootParam.InitAsDescriptorTable(1, &rootRange, shaderVisibily);
			}
		}

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(device->GetNative()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(rootParameters.size(), rootParameters.data(), 0, nullptr, Flags);

		HRESULT result;

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		if (FAILED(result) || error)
		{
			std::cout << "Failed to serialize root signature" << std::endl;
			std::cout << "Root signature Error: " << (char*)error->GetBufferPointer() << std::endl;
		}

		result = device->GetNative()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature));
		if (FAILED(result))
			std::cout << "Failed to create root signature" << std::endl;

#ifdef _DEBUG

		wchar_t lName[256];
		swprintf_s(lName, 256, L"%hs", name.c_str());
		rootSignature->SetName(lName);

#endif // _DEBUG

	}
	RootSignature::~RootSignature()
    {
		rootSignature.Reset();
		_device.reset();
    }
}