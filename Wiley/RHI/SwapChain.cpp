#include "SwapChain.h"
#include <dxgi1_5.h>

namespace RHI
{
    SwapChain::SwapChain(Device::Ref device, CommandQueue::Ref commandQueue, HWND hwnd)
        :_device(device), hwnd(hwnd)
    {
        HRESULT result;

        RECT windowRect{};
        if(GetClientRect(hwnd, &windowRect))
        {
            width = windowRect.right - windowRect.left;
            height = windowRect.bottom - windowRect.top;
        }
        else {
            std::cout << "Failed to get window size using fallback size.\n";
            width = 1600;
            height = 900;
        }

        //MSAA support
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels = {};
        msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        msaaQualityLevels.SampleCount = sampleCount;
        device->GetNative()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &msaaQualityLevels, sizeof(msaaQualityLevels));
        qualityLevels = msaaQualityLevels.NumQualityLevels;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
        swapChainDesc.BufferCount = FRAMES_IN_FLIGHT;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        //swapChainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Height = height;
        swapChainDesc.Width = width;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        ComPtr<IDXGISwapChain1> intermediateSwapChain;
        device->GetFactory()->CreateSwapChainForHwnd(commandQueue->GetQueue(), hwnd, &swapChainDesc,
            nullptr, nullptr, intermediateSwapChain.GetAddressOf());
        ThrowIfFailed(intermediateSwapChain.As(&swapChain));

        frameIndex = swapChain->GetCurrentBackBufferIndex();

        rtvHeap = std::make_shared<DescriptorHeap>(device, DescriptorHeapType::RenderTarget, FRAMES_IN_FLIGHT);

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            backBuffer[i] = std::make_shared<Texture>(TextureUsage::Present);

            swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer[i]->GetResourceAddress()));
            backBuffer[i]->SetName("SwapChainBuffer" + std::to_string(i));

            device->GetNative()->CreateRenderTargetView(backBuffer[i]->GetResource(), nullptr, rtvHeap->GetCPUHandle(i));
            descriptors[i] = DescriptorHeap::Descriptor(rtvHeap.get(), i);
        }
        
        //swapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
        
    }
    SwapChain::~SwapChain()
    {
        swapChain.Reset();
        rtvHeap.reset();
        _device.reset();
    }
    void RHI::SwapChain::Transition(CommandList::Ref commandList)
    {
        
    }
    void SwapChain::Present(bool vsync)
    {
        HRESULT result = swapChain->Present(vsync, 0);
        if (FAILED(result))
            std::cout << "Failed to present swap chain.\n";
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        this->width = width;
        this->height = height;

        // ComPtr handles release automatically - just reset
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            backBuffer[i].reset(); // Release Texture wrapper
        }

        HRESULT result = swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(result))
        {
            std::cout << "Failed to resize Swap Chain: 0x" << std::hex << result << std::endl;
            return;
        }

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            backBuffer[i] = std::make_shared<Texture>(TextureUsage::Present);

            result = swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer[i]->GetResourceAddress()));
            if (FAILED(result)) {
                std::cout << "Failed to get buffer " << i << std::endl;
                continue;
            }

            backBuffer[i]->SetName("SwapChainBuffer" + std::to_string(i));
            _device->GetNative()->CreateRenderTargetView(backBuffer[i]->GetResource(), nullptr, rtvHeap->GetCPUHandle(i));
        }
    }
}