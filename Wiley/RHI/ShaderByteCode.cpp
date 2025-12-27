#include "ShaderByteCode.h"


namespace RHI
{

	std::wstring GetEntryPoint(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:return L"VSmain";
		case ShaderType::Pixel:return L"PSmain";
        case ShaderType::Compute:return L"CSmain";
		}
	}

	std::wstring GetTarget(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:return L"vs_6_0";
		case ShaderType::Pixel:return L"ps_6_0";
        case ShaderType::Compute:return L"cs_6_0";
		}
	}

    bool ShaderCompiler::CompileShader(ShaderByteCode& byteCode)
    {
        // Create DXC interfaces
        ComPtr<IDxcUtils> utils;
        ComPtr<IDxcCompiler3> compiler;
        ComPtr<IDxcIncludeHandler> includeHandler;

        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        utils->CreateDefaultIncludeHandler(&includeHandler);

        // Load HLSL source
        ComPtr<IDxcBlobEncoding> sourceBlob;
        utils->LoadFile(byteCode.fileName.c_str(), nullptr, &sourceBlob);

        DxcBuffer sourceBuffer = {};
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();
        sourceBuffer.Encoding = DXC_CP_ACP; // Auto-detect encoding

        //std::wstring entryPoint = GetEntryPoint(byteCode.type);
        std::wstring target = GetTarget(byteCode.type);
#ifdef _DEBUG

        std::vector<LPCWSTR> args = {
            byteCode.fileName.c_str(),
            L"-E", byteCode.entryPoint.c_str(),
            L"-T", target.c_str(),
            L"-Od",             
            L"-Zi",
            L"-Qembed_debug",
            L"-Zpr",
            L"-Ges"
        };
#else
        std::vector<LPCWSTR> args = {
            byteCode.fileName.c_str(),
            L"-E", byteCode.entryPoint.c_str(),
            L"-T", target.c_str(),
            L"-O3",
            L"-Zpr",
            L"-Ges",
            L"-Zi",
            L"-Qembed_debug"
        };
#endif

        // Compile
        ComPtr<IDxcResult> result;
        compiler->Compile(
            &sourceBuffer,
            args.data(),
            (uint32_t)args.size(),
            includeHandler.Get(),
            IID_PPV_ARGS(&result)
        );

        // Print any errors
        ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetStringLength() > 0)
            std::cerr << errors->GetStringPointer() << std::endl;

        // Check status
        HRESULT status;
        result->GetStatus(&status);
        if (FAILED(status))
        {
            std::wcerr << L"Shader compilation failed: " << byteCode.fileName << std::endl;
            WILEY_DEBUGBREAK;
            return false;
        }

   
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&byteCode.byteCode), nullptr);

        std::wcout << L"Compiled: " << byteCode.fileName << std::endl;
        return true;
    }

    ShaderByteCode ShaderCompiler::CompileShader(ShaderType type, std::wstring fileName, std::wstring entryPoint, bool* result)
    {
        ShaderByteCode byteCode{
            .type = type,
            .fileName = fileName,
            .entryPoint = entryPoint
        };

        bool ret = CompileShader(byteCode);
        if (result != nullptr)
            *result = ret;
        return byteCode;
    }
	
	ID3D12ShaderReflection* ShaderCompiler::GetReflection(const ShaderByteCode& byteCode, D3D12_SHADER_DESC& desc)
	{
		IDxcUtils* utils;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

        DxcBuffer shaderBuffer{};
		shaderBuffer.Ptr = byteCode.byteCode->GetBufferPointer();
		shaderBuffer.Size = byteCode.byteCode->GetBufferSize();
        shaderBuffer.Encoding = 0;
		ID3D12ShaderReflection* shaderReflection;
		HRESULT result = utils->CreateReflection(&shaderBuffer, IID_PPV_ARGS(&shaderReflection));
		if (FAILED(result))
		{
			std::wcout << "Failed to get shader reflection. Shader Name: "
				<< byteCode.fileName.c_str() << std::endl;
			utils->Release();
            WILEY_DEBUGBREAK;
			return nullptr;
		}

		shaderReflection->GetDesc(&desc);
		utils->Release();
		return shaderReflection;
	}
}