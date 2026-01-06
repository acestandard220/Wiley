#pragma once

//Created On 4/11/2026 at 1:32am

#include <vector>
#include <cstdint>
#include <cmath>


#include <d3d.h>
#include <d3d12.h>
#include <wrl.h> //ComPtr stuff
#include <dxgi1_6.h> //DXGI stuff
#include <d3dx12.h>
#include <DirectXMath.h> //Math stuff
#include <d3dcompiler.h> //Compile shaders
#include <DirectXColors.h>
#include <dxgidebug.h>
#include <d3d12sdklayers.h>

#include <MathHelper.h>
#include <DXSampleHelper.h>

#include <Windows.h>
#include <windowsx.h>
#include <iostream>

#include <optick.h>

using namespace Microsoft::WRL;

namespace RHI
{
	enum class FeaturesSupported 
	{
		FeatureSupported_HDR
	};

	class Device
	{
	public:
		using Ref = std::shared_ptr<Device>;

		Device();
		~Device();

		[[nodiscard]] static Ref CreateDevice();

		ID3D12Device6* GetNative() { return device.Get(); }
		IDXGIFactory4* GetFactory() { return dxgiFactory.Get(); }

		bool CheckHDRSupport();

		void GetRemoveReason();


		bool CheckSupport()
		{
			D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_6 }; // Target model

			device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
			if (shaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6)
			{
				std::cout << "Supported\n";
				return true;
			}
			return false;
		}


	private:
		ComPtr<ID3D12Device6> device;
		ComPtr<IDXGIFactory4> dxgiFactory;
		ComPtr<ID3D12Debug> debugControl;
		ComPtr<IDXGIDebug1> dxgiDebugControl;
		ComPtr<IDXGIAdapter> adapter;
	};

}
