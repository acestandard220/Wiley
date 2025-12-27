#include "Device.h"
#include <fstream>

namespace RHI
{
    void Device::CollectDebugMessages()
    {
#ifdef _DEBUG
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (FAILED(device.As(&infoQueue)))
            return;

        UINT64 count = infoQueue->GetNumStoredMessages();

        for (UINT64 i = 0; i < count; i++)
        {
            SIZE_T size = 0;
            infoQueue->GetMessage(i, nullptr, &size); // Get required size

            std::vector<char> buffer(size);
            auto* msg = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());

            infoQueue->GetMessage(i, msg, &size);

            debugMessage += msg->pDescription;
            debugMessage += "\n";

            std::ofstream file("log.txt");
            if (!file.is_open())
            {
                std::cout << "Failed to open log file." << std::endl;
                return;
            }

            file << debugMessage;

        }

        infoQueue->ClearStoredMessages();
#endif
    }

    Device::Device()
    {
        HRESULT result;
        UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
        // Enable D3D12 Debug Layer
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugControl))))
        {
            debugControl->EnableDebugLayer();

            ComPtr<ID3D12Debug1> debugExt;
            debugControl.As(&debugExt);
            debugExt->SetEnableGPUBasedValidation(TRUE);

            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        }
        else {
            std::cout << "Failed to create D3D12 debug interface.\n";
        }
#endif

        // Create DXGI Factory
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

        // Select adapter 0
        dxgiFactory->EnumAdapters(0, adapter.GetAddressOf());

        ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

        DXGI_ADAPTER_DESC adapterDesc;
        adapter->GetDesc(&adapterDesc);

        std::wstring cardNameWide = adapterDesc.Description;
        std::string cardName(cardNameWide.begin(), cardNameWide.end());

        std::cout << "GPU In Use: " << cardName << std::endl;

#ifdef _DEBUG
        std::cout << "Available Video RAM: " << adapterDesc.DedicatedVideoMemory / (1024.0f * 1024.0f * 1024.0f) << " GB\n";
        std::cout << "Available System RAM: " << adapterDesc.DedicatedSystemMemory / (1024.0f * 1024.0f * 1024.0f) << " GB\n";
        std::cout << "Available Shared RAM: " << adapterDesc.SharedSystemMemory / (1024.0f * 1024.0f * 1024.0f) << " GB\n";
        std::cout << "===================  ===================  ===================  ==================="<< std::endl;
#endif

        device->SetName(L"DirectX 12 Device");
    }

    Device::~Device()
    {
        GetRemoveReason();
        CollectDebugMessages();
    }

   
    Device::Ref Device::CreateDevice()
    {
        return std::make_shared<Device>();
    }
    void Device::GetRemoveReason()
    {
        device->GetDeviceRemovedReason();
    }
}
