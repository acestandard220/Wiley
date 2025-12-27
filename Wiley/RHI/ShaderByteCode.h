#pragma once
#include "../Core/defines.h"
#include "../Core/Utils.h"

#include "Device.h"
#include "dxcapi.h"
#include "d3d12shader.h"


#include <functional>



namespace RHI
{
	enum class ShaderType
	{
		Vertex,
		Pixel,
		Compute
	};

	//Vertex Shaders must have an entry point VSmain and pixel shaders PSmain.
	struct ShaderByteCode
	{
		ShaderType type;
		std::wstring fileName;
		std::wstring entryPoint;
		ComPtr<IDxcBlob> byteCode;

		~ShaderByteCode() {
			//byteCode->Release();
		}
	};

	class ShaderCompiler
	{
	public:
		
		static bool CompileShader(ShaderByteCode& desc);
		static ShaderByteCode CompileShader(ShaderType type, std::wstring fileName, std::wstring entryPoint, bool* result);
		

		static ID3D12ShaderReflection* GetReflection(const ShaderByteCode& byteCode, D3D12_SHADER_DESC& desc);


	private:

	};
}

namespace std {
	template <>
	struct hash<RHI::ShaderType> {
		std::size_t operator()(const RHI::ShaderType& k) const noexcept {
			return static_cast<std::size_t>(k);
		}
	};
}