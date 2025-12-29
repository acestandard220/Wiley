#include "RenderContext.h"
#include "DDSTextureLoader.h"
#include "../Resource/ResourceCache.h"

namespace RHI
{
    RenderContext::RenderContext(Wiley::Window::Ref window)
        :window(window)
    {
        ZoneScopedN("RenderContext::RenderContext");

        device = Device::CreateDevice();

        gfxCommandQueue = CommandQueue::CreateCommandQueue(device, CommandListType::CommandListDirect, "Graphics_Command_Queue");
        copyCommandQueue = CommandQueue::CreateCommandQueue(device, CommandListType::CommandListCopy, "Copy_Command_Queue");
        computeCommandQueue = CommandQueue::CreateCommandQueue(device, CommandListType::CommandListCompute, "Compute_Command_Queue");

        swapChain = SwapChain::CreateSwapChain(device, gfxCommandQueue, window->GetHWND());
        frameIndex = swapChain->GetSwapChain()->GetCurrentBackBufferIndex();

        heaps.rtv = std::make_shared<DescriptorHeap>(device, DescriptorHeapType::RenderTarget, 30);
        heaps.dsv = std::make_shared<DescriptorHeap>(device, DescriptorHeapType::DepthTarget, 30 + (MAX_LIGHTS));
        heaps.cbv_srv_uav = std::make_shared<DescriptorHeap>(device, DescriptorHeapType::ShaderResource, 1000 + (MAX_LIGHTS * 2) + (MAX_IMAGETEXTURE_COUNT * 4));
        heaps.sampler = std::make_shared<DescriptorHeap>(device, DescriptorHeapType::Sampler, 32);

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            gfxCommandList[i] = CommandList::CreateCommandList(device, CommandListType::CommandListDirect, "Graphics_Command_List_" + std::to_string(i));
            gfxFence[i] = Fence::CreateFence(device, "Graphics_Fence_" + std::to_string(i));
            copyFence[i] = Fence::CreateFence(device,"Copy_Fence_" + std::to_string(i));
        }

        copyCommandList = CommandList::CreateCommandList(device, CommandListType::CommandListCopy, "Copy_Command_List");
        computeCommandList = CommandList::CreateCommandList(device, CommandListType::CommandListCompute, "Compute_Command_List");
        dynamicResourceCommandList = CommandList::CreateCommandList(device, CommandListType::CommandListCompute, "DynamicCommandList");
        dynamicResourceGfxList = CommandList::CreateCommandList(device, CommandListType::CommandListDirect, "DynamicGfxList");

        computeFence = Fence::CreateFence(device,"Compute_Fence");
        dynamicResourceFence = Fence::CreateFence(device, "DynamicResourceFence");
        dynamicResourceGfxFence = Fence::CreateFence(device, "DynamicResourceGfxFence");

        linearSampler = CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear);

        {
            RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
                L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/equirec_to_cubemap.hlsl", L"CSmain", nullptr);

            RHI::ComputePipelineSpecs cSpecs{};
            cSpecs.computeByteCode = computeByteCode;

            cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 0, 1, 0, RHI::ShaderVisibility::Compute });
            cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1, 1, 0, RHI::ShaderVisibility::Compute });
            cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 2, 1, 0, RHI::ShaderVisibility::Compute });

            environmentMapCreationPSO = RHI::ComputePipeline::CreateComputePipeline(device, cSpecs, "EquiRecToCubeMapPSO");
        }

        {
            RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
                L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/hdri_convolute.hlsl", L"Convolute", nullptr);

            RHI::ComputePipelineSpecs cSpecs{};
            cSpecs.computeByteCode = computeByteCode;

            cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 0, 1, 0, RHI::ShaderVisibility::Compute });
            cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1, 1, 0, RHI::ShaderVisibility::Compute });
            cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 2, 1, 0, RHI::ShaderVisibility::Compute });

            cubeMapConvolutePSO = RHI::ComputePipeline::CreateComputePipeline(device, cSpecs, "CubeMapConvolutePSO");
        }
    }

    RenderContext::~RenderContext()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
            gfxCommandList[i].reset();

        swapChain.reset();
        gfxCommandQueue.reset();
        device.reset();
    }

    void RenderContext::Resize(uint32_t width, uint32_t height)
    {
        ZoneScopedN("RenderContext::Resize");

        WaitForGPU();

        if (swapChain)
        {
            swapChain->Resize(width, height);
        }
    }

    void RenderContext::Present(bool vsync)
    {
        swapChain->Present(vsync);
    }

    void RenderContext::WaitForGPU()
    {
        gfxCommandQueue->Wait(gfxFence[frameIndex]);
    }

    void RenderContext::NewFrame()
    {
        frameIndex = swapChain->AcquireImage();

        gfxFence[frameIndex]->BlockCPU();
    }

    WILEY_NODISCARD IndirectCommandBuffer::Ref RenderContext::CreateIndirectCommandBuffer(IndirectCommandBufferDesc indirectCommandBufferDesc, const std::string& name)
    {
        return IndirectCommandBuffer::CreateIndirectCommandBuffer(device, indirectCommandBufferDesc, heaps, name);
    }

    Texture::Ref RenderContext::CreateShaderResourceTexture(void* data, int width, int height, int nChannel, int bitPerChannel)
    {
        const UINT nPixels = width * height;
        const UINT compSize = bitPerChannel / 8;
        const UINT size = nPixels * nChannel * compSize;

        Buffer::Ref uploadBuffer = CreateUploadBuffer(size, width * nChannel);
        
        uploadBuffer->UploadData(data, size, 0, 0);

        Texture::Ref defaultTexture = std::make_shared<Texture>(device, Texture::GetTextureFormat(bitPerChannel),
            width, height, TextureUsage::Common);

        //Wait till the last texture has operation is done.
        dynamicResourceFence->BlockCPU();

        dynamicResourceCommandList->Begin();
        dynamicResourceCommandList->ImageBarrier(defaultTexture, RHI::TextureUsage::CopyDest);
        dynamicResourceCommandList->CopyBufferToTexture(uploadBuffer, defaultTexture);
        dynamicResourceCommandList->ImageBarrier(defaultTexture, RHI::TextureUsage::Common);
        dynamicResourceCommandList->End();

        computeCommandQueue->Submit({ dynamicResourceCommandList });
        dynamicResourceFence->Signal(computeCommandQueue.get());
        dynamicResourceFence->BlockCPU();

        return defaultTexture;
    }

    WILEY_NODISCARD Texture::Ref RenderContext::CreateShaderResourceTextureFromFile(const std::wstring& name, int& width, int& height, int& nChannel, int& bpc)
    {
        dynamicResourceGfxFence->BlockCPU();

        Texture::Ref uploadTexture  = std::make_shared<Texture>(RHI::TextureUsage::CopySrc);
        Texture::Ref defaultTexture = std::make_shared<Texture>(RHI::TextureUsage::Common);

        dynamicResourceGfxList->Begin();

        HRESULT result = DirectX::CreateDDSTextureFromFile12(device->GetNative(), dynamicResourceGfxList->GetCommandList(), name.c_str(), defaultTexture->GetComPtr(), uploadTexture->GetComPtr());
        if (FAILED(result)) {
            return nullptr;
        }

        D3D12_RESOURCE_DESC desc = defaultTexture->GetResource()->GetDesc();
        defaultTexture->width = static_cast<UINT>(desc.Width);
        defaultTexture->height = desc.Height;
        defaultTexture->format = desc.Format;
        defaultTexture->_device = device;

        width = static_cast<UINT>(desc.Width);
        height = desc.Height;

        switch (desc.Format) {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                nChannel = 4; bpc = 8; break;

            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                nChannel = 4; bpc = 8; break;

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                nChannel = 4; bpc = 16; break;

            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                nChannel = 4; bpc = 32; break;

            case DXGI_FORMAT_R8_UNORM:
                nChannel = 1; bpc = 8; break;

            case DXGI_FORMAT_R16_FLOAT:
                nChannel = 1; bpc = 16; break;

            case DXGI_FORMAT_BC1_UNORM: 
            case DXGI_FORMAT_BC2_UNORM: 
            case DXGI_FORMAT_BC3_UNORM: 
                nChannel = 4; bpc = 0;  
                break;
        }

        dynamicResourceGfxList->End();

        gfxCommandQueue->Submit({ dynamicResourceGfxList });
        dynamicResourceGfxFence->Signal(gfxCommandQueue.get());
        dynamicResourceGfxFence->BlockCPU();
        return defaultTexture;
    }

    std::vector<uint8_t> RenderContext::GetTextureBytes(RHI::Texture::Ref texture) {
        UINT width = texture->GetWidth();
        UINT height = texture->GetHeight();
        UINT pixelSize = Texture::GetPixelSize(texture->GetFormat());

        UINT rowPitch = (width * pixelSize + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
            & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

        Buffer::Ref readbackBuffer = std::make_shared<Buffer>(device, RHI::BufferUsage::ReadBack, true, rowPitch * height, rowPitch, "TextureReadback");

        dynamicResourceCommandList->Begin({ heaps.cbv_srv_uav });
        auto currentState = texture->GetState();
        dynamicResourceCommandList->ImageBarrier(texture, RHI::TextureUsage::CopySrc);
        dynamicResourceCommandList->CopyTextureToBuffer(texture, readbackBuffer);
        dynamicResourceCommandList->ImageBarrier(texture, currentState);
        dynamicResourceCommandList->End();

        computeCommandQueue->Submit({ dynamicResourceCommandList });
        dynamicResourceFence->Signal(computeCommandQueue.get());
        dynamicResourceFence->BlockCPU();

        std::vector<uint8_t> textureBytes(width * height * pixelSize);
        std::vector<uint8_t> stagingData(rowPitch * height);
        std::span<uint8_t> stagingSpan(stagingData);
        readbackBuffer->ReadData<uint8_t>(stagingSpan);

        for (UINT y = 0; y < height; ++y) {
            memcpy(&textureBytes[y * width * pixelSize],
                &stagingData[y * rowPitch],
                width * pixelSize);
        }

        return textureBytes;
    }

    WILEY_NODISCARD CubeMap::Ref RenderContext::CreateShaderResourceCubeMap(float* data, uint32_t width, uint32_t height, int nChannel, int bitPerChannel, const std::string& name)
    {
        const UINT size = height * width * nChannel * (bitPerChannel / 8);

        const UINT cubeFaceSize = width / 4;

        RHI::CubeMap::Ref cubeMap = CreateCubeMap(cubeFaceSize, cubeFaceSize, Texture::GetTextureFormat(bitPerChannel), name);
        RHI::Texture::Ref texture = CreateShaderResourceTexture(data, width, height, nChannel, 32);
        RHI::DescriptorHeap::Descriptor textureSRV = heaps.cbv_srv_uav->Allocate();
        if (!textureSRV.valid) {
            std::cout << "Failed to allocate SRV for HDR image on Cubemap Creation.\n" << std::endl;
            return nullptr;
        }

        texture->BuildSRV(textureSRV);
        
        dynamicResourceFence->BlockCPU();
        dynamicResourceCommandList->Begin({ heaps.cbv_srv_uav,heaps.sampler });

        {
            dynamicResourceCommandList->SetComputePipeline(environmentMapCreationPSO);
            dynamicResourceCommandList->SetComputeRootSignature(environmentMapCreationPSO->GetRootSignature());

            dynamicResourceCommandList->CubeMapBarrier(cubeMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            dynamicResourceCommandList->BindComputeShaderResource(cubeMap->GetUAV(), 0);
            dynamicResourceCommandList->BindComputeShaderResource(textureSRV, 1);
            dynamicResourceCommandList->BindComputeSamplerResource(linearSampler->GetDescriptor(), 2);

            dynamicResourceCommandList->Dispatch(width / 8, height / 8, 6);

            dynamicResourceCommandList->CubeMapBarrier(cubeMap, D3D12_RESOURCE_STATE_COMMON);
        }

        dynamicResourceCommandList->End();
        computeCommandQueue->Submit({ dynamicResourceCommandList });
        dynamicResourceFence->Signal(computeCommandQueue.get());
        dynamicResourceFence->BlockCPU(); 

        return cubeMap;
    }

    WILEY_NODISCARD CubeMap::Ref RenderContext::ConvoluteCubeMap(CubeMap::Ref cubmap, uint32_t convSize, const std::string& name)
    {
        uint32_t width = convSize;
        uint32_t height = convSize;

        CubeMap::Ref convoluteMap = CreateCubeMap(width, height, TextureFormat::RGBA32, name);

        dynamicResourceFence->BlockCPU();
        dynamicResourceCommandList->Begin({ heaps.cbv_srv_uav,heaps.sampler });

        {

            dynamicResourceCommandList->SetComputePipeline(cubeMapConvolutePSO);
            dynamicResourceCommandList->SetComputeRootSignature(cubeMapConvolutePSO->GetRootSignature());

            dynamicResourceCommandList->CubeMapBarrier(convoluteMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);


            dynamicResourceCommandList->BindComputeShaderResource(convoluteMap->GetUAV(), 0);
            dynamicResourceCommandList->BindComputeShaderResource(cubmap->GetSRV(), 1);
            dynamicResourceCommandList->BindComputeSamplerResource(linearSampler->GetDescriptor(), 2);

            dynamicResourceCommandList->Dispatch(width / 8, height / 8, 6);

            dynamicResourceCommandList->CubeMapBarrier(convoluteMap, D3D12_RESOURCE_STATE_COMMON);
        }

        dynamicResourceCommandList->End();
        computeCommandQueue->Submit({ dynamicResourceCommandList });
        dynamicResourceFence->Signal(computeCommandQueue.get());
        dynamicResourceFence->BlockCPU();
        return convoluteMap;
    }

    Texture::Ref RenderContext::CreateTexture(TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage,const std::string& name)
    {
        return std::make_shared<Texture>(device, format, width, height, usage, heaps, name);
    }

    Buffer::Ref RenderContext::CreateUploadBuffer(uint64_t size, uint64_t stride,const std::string& name)
    {
        return std::make_shared<Buffer>(device, BufferUsage::Copy, true, size, stride, name, nullptr);
    }

    Buffer::Ref RenderContext::CreateComputeStorageBuffer(uint64_t size, uint64_t stride, const std::string& name)
    {
        return std::make_shared<Buffer>(device, BufferUsage::ComputeStorage, false, size, stride, name, heaps.cbv_srv_uav);
    }

    WILEY_NODISCARD Buffer::Ref RenderContext::CreateIndexBuffer(uint64_t size, uint64_t stride, const std::string& name)
    {
        return std::make_shared<Buffer>(device, BufferUsage::Index, false, size, stride, name, nullptr);
    }

    Buffer::Ref RenderContext::CreateVertexBuffer(uint64_t size, uint64_t stride, const std::string& name)
    {
        return std::make_shared<Buffer>(device, BufferUsage::Vertex, false, size, stride, name, nullptr);
    }

    Buffer::Ref RenderContext::CreateConstantBuffer(bool persistent, uint64_t size, const std::string& name)
    {
        return std::make_shared<Buffer>(device, BufferUsage::Constant, persistent, size,0, name, heaps.cbv_srv_uav);
    }

    GraphicsPipeline::Ref RenderContext::CreateGraphicsPipeline(GraphicsPipelineSpecs specs)
    {
        return std::make_shared<GraphicsPipeline>(device, specs);
    }

    RootSignature::Ref RenderContext::CreateRootSignature(RootSignatureSpecs specs, const std::string& name)
    {
        return std::make_shared<RootSignature>(device, specs, name); 
    }

    Sampler::Ref RenderContext::CreateSampler(SamplerAddress address, SamplerFilter filter)
    {
        return std::make_shared<Sampler>(device, address, filter, heaps);
    }

    WILEY_NODISCARD CubeMap::Ref RenderContext::CreateCubeMap(uint32_t width, uint32_t height, TextureFormat formate, const std::string& name) {
        return std::make_shared<CubeMap>(device, formate, width, height, heaps, name);
    }

    void RenderContext::BindGraphicsPipeline(GraphicsPipeline::Ref graphicsPipeline)
    {
        auto commandList = GetCurrentCommandList();
        commandList->SetGraphicsPipeline(graphicsPipeline);
        commandList->SetGraphicsRootSignature(graphicsPipeline->GetRootSignature());
    }

    void RenderContext::ExecuteGraphicsCommandList(const std::vector<CommandList::Ref> list)
    {
        ZoneScopedN("RenderContext::ExecuteGraphicsCommandList");

        gfxCommandQueue->Submit(list);
    }

    void RenderContext::ExecuteCopyCommandList(const std::vector<CommandList::Ref> list)
    {
        copyCommandQueue->Submit(list);
    }

    void RenderContext::ExecuteComputeCommandList(const std::vector<CommandList::Ref> list)
    {
        computeCommandQueue->Submit(list);
    }

    void RenderContext::TransitionSwapchain()
    {
        Texture::Ref currentTexture = GetBackBufferTexture();
        auto currentState = currentTexture->GetState();

        GetCurrentCommandList()->ImageBarrier(currentTexture, (currentState == TextureUsage::Present) ? TextureUsage::RenderTarget : TextureUsage::Present);
    }

    DescriptorHeap::Descriptor RenderContext::AllocateCBV_SRV_UAV() const
    {
        DescriptorHeap::Descriptor descriptor = heaps.cbv_srv_uav->Allocate();
        if (!descriptor.valid) {
            std::cout << "Failed to allocate space for CBV_SRV_UAV Descriptor. Check maximum size specified on initialize of parent heap."
                << std::endl;
            return DescriptorHeap::Descriptor::Invalid();
        }

        return descriptor;
    }

    std::vector<DescriptorHeap::Descriptor> RenderContext::AllocateCBV_SRV_UAV(UINT nDescriptors) const
    {
        return heaps.cbv_srv_uav->AllocateNullDescriptor(nDescriptors);
    }

    std::vector<DescriptorHeap::Descriptor> RenderContext::AllocateDSV(UINT nDescriptors) const
    {
        return heaps.dsv->AllocateNullDescriptor(nDescriptors);
    }

    DescriptorHeap::Descriptor RenderContext::AllocateSampler() const
    {
        DescriptorHeap::Descriptor descriptor = heaps.sampler->Allocate();
        if (!descriptor.valid) {
            std::cout << "Failed to allocate space for Sampler Descriptor. Check maximum size specified on initialize of parent heap."
                << std::endl;
            return DescriptorHeap::Descriptor::Invalid();
        }
        return descriptor;
    }

    DescriptorHeap::Descriptor RenderContext::AllocateRTV() const
    {
        DescriptorHeap::Descriptor descriptor = heaps.rtv->Allocate();
        if (!descriptor.valid) {
            std::cout << "Failed to allocate space for Render Target View Descriptor. Check maximum size specified on initialize of parent heap."
                << std::endl;
            return DescriptorHeap::Descriptor::Invalid();
        }
        return descriptor;
    }

    DescriptorHeap::Descriptor RenderContext::AllocateDSV() const
    {
        DescriptorHeap::Descriptor descriptor = heaps.dsv->Allocate();
        if (!descriptor.valid) {
            std::cout << "Failed to allocate space for DSV Descriptor. Check maximum size specified on initialize of parent heap."
                << std::endl;
            return DescriptorHeap::Descriptor::Invalid();
        }
        return descriptor;
    }

    DescriptorHeap::Descriptor RenderContext::AllocateSRV() const
    {
        return AllocateCBV_SRV_UAV();
    }

    DescriptorHeap::Descriptor RenderContext::AllocateUAV() const
    {
        return AllocateCBV_SRV_UAV();
    }

    DescriptorHeap::Descriptor RenderContext::AllocateCBV() const
    {
        return AllocateCBV_SRV_UAV();
    }

}