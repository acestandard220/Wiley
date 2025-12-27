#include "FrameGraph.h"


namespace Renderer3D
{
    FrameGraph::FrameGraph(RHI::RenderContext::Ref rctx)
        :rctx(rctx)
    {

    }
    
    ResourceHandle FrameGraph::CreateTextureResource(const std::string& name, const TextureResourceDesc& resourceDesc, RHI::TextureUsage usage, bool isScreenSizeDependent)
    {
        if (strDesc.find(name) != strDesc.end())
            return { (uint32_t)strDesc[name] };

        auto resource = rctx->CreateTexture(resourceDesc.format, resourceDesc.width,
            resourceDesc.height, resourceDesc.usage, name);

        uint32_t index = resources.size();
        RenderPassResource passResource = {
            .resource = resource,
            .type = PassResourceType::Texture,
            .isScreenSizeDependent = isScreenSizeDependent
        };
        resources.push_back(passResource);

        ResourceHandle handle = {
            .id = index,
            .usage = usage,
            .creationState = resourceDesc.usage
        };

        strDesc[name] = index;
        return handle;
    }

    ResourceHandle FrameGraph::CreateBufferResource(const std::string& name,const BufferResourceDesc& resourceDesc, RHI::BufferUsage usage) {
        if (strDesc.find(name) != strDesc.end())
            return { (uint32_t)strDesc[name] };

        auto resource = std::make_shared<RHI::Buffer>(rctx->GetDevice(), resourceDesc.usage,
            resourceDesc.persistent, resourceDesc.size, resourceDesc.stride, name, rctx->GetDescriptorHeaps().cbv_srv_uav);

        uint32_t index = resources.size();
        RenderPassResource  passResource = {
            .resource = resource,
            .type = PassResourceType::Buffer
        };
        resources.push_back(passResource);

        ResourceHandle handle = {
            .id = index,
            .usage = usage,
            .creationState = resourceDesc.usage
        };

        strDesc[name] = index;
        return handle;
    }

    ResourceHandle FrameGraph::ImportTextureResource(const std::string& name, RHI::Texture::Ref texture, RHI::TextureUsage usage)
    {
        if (strDesc.find(name) != strDesc.end())
            return { (uint32_t)strDesc[name],usage };

        uint32_t index = resources.size();
        RenderPassResource passResource = {
            .resource = texture,
            .type = PassResourceType::Texture
        };
        resources.push_back(passResource);

        ResourceHandle handle = {
            .id = index,
            .usage = usage
        };

        strDesc[name] = index;
        return handle;
    }

    ResourceHandle FrameGraph::ImportBufferResource(const std::string& name, RHI::Buffer::Ref buffer, RHI::BufferUsage usage)
    {
        if (strDesc.find(name) != strDesc.end())
            return { (uint32_t)strDesc[name], usage };

        uint32_t index = resources.size();
        RenderPassResource passResource = {
            .resource = buffer,
            .type = PassResourceType::Buffer
        };
        resources.push_back(passResource);

        ResourceHandle handle = {
           .id = index,
           .usage = usage
        };

        strDesc[name] = index;
        return handle;

    }

    ResourceHandle FrameGraph::ReadTextureResource(const std::string& name, RHI::TextureUsage usage)
    {
        if (strDesc.find(name) != strDesc.end())
        {
            auto id = strDesc[name];
            if (resources[id].type != PassResourceType::Texture) {
                std::cout << "Attempting to Read non texture resource in texture pipeline." << std::endl;
                return {};
            }
            return { (uint32_t)id ,usage };
        }

        std::cout << "Failed to find read to buffer resource. Name :: " << name << std::endl;
        return {};
    }

    ResourceHandle FrameGraph::ReadBufferResource(const std::string& name, RHI::BufferUsage usage)
    {
        if (strDesc.find(name) != strDesc.end())
        {
            auto id = strDesc[name];
            if (resources[id].type != PassResourceType::Buffer) {
                std::cout << "Attempting to Read non buffer resource in buffer pipeline." << std::endl;
                return {};
            }
            return { (uint32_t)id ,usage };
        }

        std::cout << "Failed to find read to buffer resource. Name :: " << name << std::endl;
        return {};
    }

    ResourceHandle FrameGraph::WriteTextureResource(const std::string& name, RHI::TextureUsage usage)
    {
        if (strDesc.find(name) != strDesc.end())
        {
            auto id = strDesc[name];
            if (resources[id].type != PassResourceType::Texture) {
                std::cout << "Attempting to write to a non texture resource in texture pipeline." << std::endl;
                return {};
            }
            return { (uint32_t)id,usage };
        }

        std::cout << "Failed to find write to texture resource. Name :: " << name << std::endl;
        return {};
    }

    ResourceHandle FrameGraph::WriteBufferResource(const std::string& name, RHI::BufferUsage usage)
    {
        if (strDesc.find(name) != strDesc.end())
        {
            auto id = strDesc[name];
            if (resources[id].type != PassResourceType::Buffer) {
                std::cout << "Attempting to write to a non buffer resource in buffer pipeline." << std::endl;
                return {};
            }
            return { (uint32_t)id, usage };
        }

        std::cout << "Failed to find write to buffer resource. Name :: " << name << std::endl;
        return {};
    }

    RenderPassResource FrameGraph::GetResource(const std::string& name)
    {
        if (strDesc.find(name) != strDesc.end())
        {
            auto id = strDesc[name];
            return resources[id];
        }
        return {};
    }

    RenderPassResource FrameGraph::GetResource(const ResourceHandle& handle)
    {
        if (!handle.IsValid())
            return {};
        return resources[handle.id];
    }

    RenderPassResource Renderer3D::FrameGraph::GetResource(std::vector<ResourceHandle> const& res, int accessSlot)
    {
        if (accessSlot > res.size())
        {
            std::cout << "[ERROR:FRAMEGRAPH] :: Attempting to access invalid resource slot." << std::endl;
            return {};
        }

        return GetResource(res[accessSlot]);
    }

    RHI::Texture::Ref FrameGraph::GetInputTextureResource(RenderPass& pass, int accessSlot)
    {
        RenderPassResource resource = GetResource(pass.inputs, accessSlot);
        if (resource.type != PassResourceType::Texture) {
            std::cout << "Attempting to get non texture resource as texture resource. Invalid access slot for texture resource." << std::endl;
            return nullptr;
        }
        return std::get<RHI::Texture::Ref>(resource.resource);
    }

    RHI::Buffer::Ref FrameGraph::GetInputBufferResource(RenderPass& pass, int accessSlot)
    {
        RenderPassResource resource = GetResource(pass.inputs, accessSlot);
        if (resource.type != PassResourceType::Buffer) {
            std::cout << "Attempting to get non buffer resource as buffer resource. Invalid access slot for buffer resource." << std::endl;
            return nullptr;
        }
        return std::get<RHI::Buffer::Ref>(resource.resource);
    }

    RHI::Texture::Ref FrameGraph::GetOutputTextureResource(RenderPass& pass, int accessSlot)
    {
        RenderPassResource resource = GetResource(pass.outputs, accessSlot);
        if (resource.type != PassResourceType::Texture) {
            std::cout << "Attempting to get non texture resource as texture resource. Invalid access slot for texture resource." << std::endl;
            return nullptr;
        }
        return std::get<RHI::Texture::Ref>(resource.resource);
    }

    RHI::Buffer::Ref FrameGraph::GetOutputBufferResource(RenderPass& pass, int accessSlot)
    {
        RenderPassResource resource = GetResource(pass.outputs, accessSlot);
        if (resource.type != PassResourceType::Buffer) {
            std::cout << "Attempting to get non buffer resource as buffer resource. Invalid access slot for buffer resource." << std::endl;
            return nullptr;
        }
        return std::get<RHI::Buffer::Ref>(resource.resource);
    }

    void Renderer3D::FrameGraph::TransistionInputTextures(RenderPass& pass)
    {
        std::vector<RHI::Barrier> barriers;
        barriers.reserve(pass.inputs.size());
        for (auto& handle : pass.inputs)
        {

            try
            {
                barriers.push_back({ 
                    std::get<RHI::Texture::Ref>(GetResource(handle).resource),
                    std::get<RHI::TextureUsage>(handle.usage) 
                    });
            }
            catch(std::bad_variant_access& exception){
                //std::cout << "Attempting to transition a non texture resource with textures params." << std::endl;
                continue;
            }
        }

        rctx->GetCurrentCommandList()->ImageBarrier(barriers);
    }

    void FrameGraph::TransitionOutputTextures(RenderPass& pass)
    {
        std::vector<RHI::Barrier> barriers;
        barriers.reserve(pass.outputs.size());
        for (auto& handle : pass.outputs)
        {
            try
            {
                barriers.push_back({
                    std::get<RHI::Texture::Ref>(GetResource(handle).resource),
                    std::get<RHI::TextureUsage>(handle.usage) 
                    });
            }
            catch (std::bad_variant_access& exception) {
                //std::cout << "Attempting to transition a non texture resource with textures params." << std::endl;
                continue;
            }
        }

        rctx->GetCurrentCommandList()->ImageBarrier(barriers);
    }

    void FrameGraph::TransitionInputBuffers(RenderPass& pass)
    {
        for (int i = 0; i < pass.inputs.size(); i++)
        {
            auto buffer = std::get<RHI::Buffer::Ref>(GetResource(pass.inputs[i]).resource);
        }

    }

    void FrameGraph::TransitionInputTextureToCreationState(RenderPass& pass)
    {
        std::vector<RHI::Barrier> barriers;
        barriers.reserve(pass.inputs.size());
        for (auto& handle : pass.inputs)
        {
            try
            {
                barriers.push_back({ 
                    std::get<RHI::Texture::Ref>(GetResource(handle).resource),
                    std::get<RHI::TextureUsage>(handle.creationState)
                    });
            }
            catch (std::bad_variant_access& exception) {
                //std::cout << "Attempting to transition a non texture resource with textures params." << std::endl;
                continue;
            }
        }

        rctx->GetCurrentCommandList()->ImageBarrier(barriers);
    }

    void FrameGraph::TransitionOutputTextureToCreationState(RenderPass& pass)
    {
        std::vector<RHI::Barrier> barriers;
        barriers.reserve(pass.outputs.size());
        for (auto& handle : pass.outputs)
        {
            try
            {
                barriers.push_back({ 
                    std::get<RHI::Texture::Ref>(GetResource(handle).resource),
                    std::get<RHI::TextureUsage>(handle.creationState) 
                    });
            }
            catch (std::bad_variant_access& exception) {
                //std::cout << "Attempting to transition a non texture resource with textures params." << std::endl;
                continue;
            }
        }

        rctx->GetCurrentCommandList()->ImageBarrier(barriers);
    }

    UINT FrameGraph::GetPassWaitValue(const std::string& name)
    {
        for (auto& sortedP : sortedPasses)
            if (sortedP->name == name)
                return sortedP->waitValue;
        return 0;
    }



    void FrameGraph::AddPass(const RenderPass& pass)
	{
		passes.push_back(pass);
	}

	void FrameGraph::Compile()
    {
        std::unordered_set<RenderPass*> skippedPass;
        //Ref to render pass that writes to a resource.
        std::unordered_map<int, RenderPass*> writeMap;
        for (auto& pass : passes)
            for (auto& write : pass.outputs)
                writeMap[write.id] = &pass;

        std::unordered_map<RenderPass*, std::vector<RenderPass*>> passDependencies;
        std::unordered_map<RenderPass*, int> indegree; //Represents the number of dependencies for a render pass

        for (auto& pass : passes)
            indegree[&pass] = 0;

        //For each pass find which other pass writes to it for us to use.
        //Then add that pass to our dependency list. Then increase our number of dependencies
        for (auto& pass : passes)
        {
            
            for (auto& input : pass.inputs)
            {
                if (writeMap.count(input.id))
                {
                    RenderPass* dependency = writeMap[input.id];
                    if (dependency != &pass)
                    {
                        passDependencies[dependency].push_back(&pass);
                        indegree[&pass]++;
                    }
                }
            }
        }

        //Kahn's Sorting algo...
        std::queue<RenderPass*> passQ;
        for (auto& [pass, nResDep] : indegree)
            if (nResDep == 0)
                passQ.push(pass);

        sortedPasses.clear();

        while (!passQ.empty())
        {
            RenderPass* pass = passQ.front();

            passQ.pop();
            sortedPasses.push_back(pass);

            for (auto& _pass : passDependencies[pass])
            {
                if (--indegree[_pass] == 0)
                    passQ.push(_pass);
            }
        }
    }

	void FrameGraph::Execute()const
	{
        ZoneScopedN("FrameGraph::Execute");

		for (auto& pass : sortedPasses)
        {
            pass->execute(*pass);
        }
	}

    void FrameGraph::OnResize(std::uint32_t width, std::uint32_t height) {
        ZoneScopedN("FrameGraph::OnResize");

        for (auto& screenSizeDeps : resources
            | std::views::filter([&](auto& res) {return res.isScreenSizeDependent; }))
        {
            RHI::Texture::Ref textureResource = std::get<RHI::Texture::Ref>(screenSizeDeps.resource);
            textureResource->Resize(width, height);
        }
    }

    std::vector<std::string_view> FrameGraph::GetPassOrder() {
        std::vector<std::string_view> ret;
        std::cout << "Render Pass (Sorted)\n";
        for (auto& pass : sortedPasses) {
            ret.push_back(pass->name);
#ifdef _DEBUG
            std::cout << "Pass :: " << pass->name << std::endl;
#endif
        }
        return ret;
    }

}