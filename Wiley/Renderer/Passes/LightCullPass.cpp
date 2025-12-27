#include "../Renderer.h"

namespace Renderer3D {

	void Renderer::ClusterGeneration(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ClusterGeneration");

		if (!camera->IsChanged() || !_scene->IsWindowResize()) {
#ifdef _DEBUG
//			std::cout << __FUNCTION__ << " has been skipped." << std::endl;
#endif // _DEBUG
			//return;
		}

		std::cout << "ClusterGen" << std::endl;

		RHI::ComputePipeline::Ref pso = computePsoCache[RenderPassSemantic::GenClusterPass];

		RHI::CommandList::Ref computeCommandList = rctx->GetComputeCommandList();
		RHI::CommandQueue::Ref computeCommandQueue = rctx->GetComputeQueue();

		RHI::Fence::Ref computeFence = rctx->GetComputeFence();

		RHI::Buffer::Ref dispatchConstants = frameGraph->GetOutputBufferResource(pass, 0);
		RHI::Buffer::Ref clusterBuffer = frameGraph->GetOutputBufferResource(pass, 1);

		struct ClusterDispatchParams
		{
			DirectX::XMUINT4 clusterCount;      
			uint32_t tileSize[2];         
			uint32_t screenDimension[2];  

			float nearPlane = 0.1f;              
			float farPlane = 1000.0f;

			DirectX::XMFLOAT3 cameraPosition;      
			float _padding0;               

			DirectX::XMFLOAT4X4 inverseProjection;  
		}constantBufferData;

		auto camera = _scene->GetCamera();
		
		UINT screenWidth = 0;
		UINT screenHeight = 0;
		window->GetClientDimensions(screenWidth, screenHeight);

		UINT clusterCountX = (screenWidth + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountY = (screenHeight + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountZ = CLUSTER_DEPTH;
		WILEY_MAYBE_UNUSED UINT clusterCount = clusterCountX * clusterCountY * clusterCountZ;

		{
			auto pos = camera->GetPosition();
			constantBufferData.cameraPosition = { pos.x,pos.y,pos.z };
			constantBufferData.clusterCount = { clusterCountX, clusterCountY, clusterCountZ, 1 };
			constantBufferData.farPlane = camera->GetFar();
			DirectX::XMStoreFloat4x4(&constantBufferData.inverseProjection, camera->GetInverseProjection());
			constantBufferData.nearPlane = camera->GetNear();
			constantBufferData.screenDimension[0] = screenWidth;
			constantBufferData.screenDimension[1] = screenHeight;
			constantBufferData.tileSize[0] = TILE_GRID_SIZE;
			constantBufferData.tileSize[1] = TILE_GRID_SIZE;

			dispatchConstants->UploadData(&constantBufferData, WILEY_SIZEOF(ClusterDispatchParams), 0, 0);
		}

		{
			computeCommandList->Begin({ rctx->GetDescriptorHeaps().cbv_srv_uav });

			computeCommandList->SetComputePipeline(pso);
			computeCommandList->SetComputeRootSignature(pso->GetRootSignature());
		}

		{
			computeCommandList->BindComputeShaderResource(dispatchConstants->GetCBV(), 0);
			computeCommandList->BindComputeShaderResource(clusterBuffer->GetUAV(), 1);
		}

		{
			computeCommandList->Dispatch(clusterCountX, clusterCountY, clusterCountZ);
		}

		{
			computeCommandList->End();
			computeCommandQueue->Submit({ computeCommandList });

			computeFence->Signal(computeCommandQueue.get());
			computeFence->BlockCPU();
		}
	}

	void Renderer::ClusterCullingPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ClusterCullingPass");

		RHI::ComputePipeline::Ref pso = computePsoCache[RenderPassSemantic::ClusterCullPass];

		RHI::CommandList::Ref computeCommandList = rctx->GetComputeCommandList();
		RHI::CommandQueue::Ref computeCommandQueue = rctx->GetComputeQueue();
		RHI::Fence::Ref computeFence = rctx->GetComputeFence();
		RHI::Fence::Ref graphicsFence = rctx->GetCurrentGraphicsFence();

		{
			ZoneScopedN("ClusterCullingPassWaitForDepthPrePass");

			UINT depthPrepassWaitValue = frameGraph->GetPassWaitValue("DepthPrepass");
			graphicsFence->GPUWaitForValue(computeCommandQueue.get(), depthPrepassWaitValue);
		}

		RHI::Texture::Ref depthMap = frameGraph->GetInputTextureResource(pass, 0);
		RHI::Buffer::Ref resetActiveClusterBuffer = frameGraph->GetInputBufferResource(pass, 1);

		RHI::Buffer::Ref clusterConstants = frameGraph->GetOutputBufferResource(pass, 0);
		RHI::Buffer::Ref activeClusterIndex = frameGraph->GetOutputBufferResource(pass, 1);

		struct ClusterCullingDispatchParams
		{
			DirectX::XMUINT4 clusterCount;     
			uint32_t tileSize[2];              
			uint32_t screenDimension[2];       
			float nearPlane;                   
			float farPlane;                    
			float _padding[2];                 
		}params;

		auto camera = _scene->GetCamera();

		UINT screenWidth = 0;
		UINT screenHeight = 0;
		window->GetClientDimensions(screenWidth, screenHeight);

		UINT clusterCountX = (screenWidth + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountY = (screenHeight + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountZ = CLUSTER_DEPTH;
		WILEY_MAYBE_UNUSED UINT clusterCount = clusterCountX * clusterCountY * clusterCountZ;

		params.clusterCount = { clusterCountX,clusterCountY,clusterCountZ,1 };
		params.farPlane =  camera->GetFar();
		params.nearPlane = camera->GetNear();
		params.screenDimension[0] = screenWidth;
		params.screenDimension[1] = screenHeight;
		params.tileSize[0] = TILE_GRID_SIZE;
		params.tileSize[1] = TILE_GRID_SIZE;

		clusterConstants->UploadData(&params, WILEY_SIZEOF(ClusterCullingDispatchParams), 0, 0);

		static bool firstReset = true;
		if (firstReset) {

			UINT* resetBufferPtr = nullptr;
			resetActiveClusterBuffer->Map(reinterpret_cast<void**>(&resetBufferPtr), 0, 0);
			memset(resetBufferPtr, 0, resetActiveClusterBuffer->GetSize());
			resetActiveClusterBuffer->Unmap(0, 0);

			firstReset = false;
		}

		{
			auto& heaps = rctx->GetDescriptorHeaps();
			computeCommandList->Begin({ heaps.cbv_srv_uav, heaps.sampler });
			computeCommandList->SetComputePipeline(pso);
			computeCommandList->SetComputeRootSignature(pso->GetRootSignature());
		}

		{
			computeCommandList->BindComputeShaderResource(clusterConstants->GetCBV(), 0);
			computeCommandList->BindComputeShaderResource(activeClusterIndex->GetUAV(), 1);
			computeCommandList->BindComputeShaderResource(depthMap->GetSRV(), 2);
			computeCommandList->BindComputeSamplerResource(sampler->GetDescriptor(), 3);
		}

		{
			computeCommandList->BufferUAVToCopyDest(activeClusterIndex);
			computeCommandList->CopyBufferToBuffer(resetActiveClusterBuffer, 0, activeClusterIndex, activeClusterIndex->GetSize(), false);
			computeCommandList->BufferCopyDestToUAV(activeClusterIndex);

			computeCommandList->Dispatch(clusterCountX, clusterCountY, 1);
		}
	}

	void Renderer::CompactClusterPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::CompactClusterPass");

		RHI::ComputePipeline::Ref pso = computePsoCache[RenderPassSemantic::CompactClusterPass];

		RHI::CommandList::Ref computeCommandList = rctx->GetComputeCommandList();
		RHI::CommandQueue::Ref computeCommandQueue = rctx->GetComputeQueue();
		RHI::Fence::Ref computeFence = rctx->GetComputeFence();

		RHI::Buffer::Ref activeClusters = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Buffer::Ref constantBuffer = frameGraph->GetInputBufferResource(pass, 1);

		RHI::Buffer::Ref activeClusterCount = frameGraph->GetOutputBufferResource(pass, 0);
		RHI::Buffer::Ref activeClusterIndex = frameGraph->GetOutputBufferResource(pass, 1);
		RHI::Buffer::Ref readBackActiveClusterCount = frameGraph->GetOutputBufferResource(pass, 2);


		struct ClusterCullingDispatchParams
		{
			DirectX::XMUINT4 clusterCount;      
			uint32_t tileSize[2];               
			uint32_t screenDimension[2];        
			float nearPlane;                    
			float farPlane;                     
			float _padding[2];                  
		}params;

		auto camera = _scene->GetCamera();

		UINT screenWidth = 0;
		UINT screenHeight = 0;
		window->GetClientDimensions(screenWidth, screenHeight);

		UINT clusterCountX = (screenWidth + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountY = (screenHeight + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountZ = CLUSTER_DEPTH;
		UINT clusterCount = clusterCountX * clusterCountY * clusterCountZ;

		params.clusterCount = { clusterCountX,clusterCountY,clusterCountZ,1 };
		params.farPlane = camera->GetFar();
		params.nearPlane = camera->GetNear();
		params.screenDimension[0] = screenWidth;
		params.screenDimension[1] = screenHeight;
		params.tileSize[0] = TILE_GRID_SIZE;
		params.tileSize[1] = TILE_GRID_SIZE;

		constantBuffer->UploadData(&params, WILEY_SIZEOF(ClusterCullingDispatchParams), 0, 0);
		
		{
			computeCommandList->SetComputePipeline(pso);
			computeCommandList->SetComputeRootSignature(pso->GetRootSignature());

			computeCommandList->BindComputeShaderResource(constantBuffer->GetCBV(), 0);
			computeCommandList->BindComputeShaderResource(activeClusters->GetUAV(), 1);
			computeCommandList->BindComputeShaderResource(activeClusterIndex->GetUAV(), 2);
			computeCommandList->BindComputeShaderResource(activeClusterCount->GetUAV(), 3);
		}

		{
			computeCommandList->ClearUAVUint(activeClusterCount);
			computeCommandList->UAVBarrier(activeClusterCount);
			computeCommandList->Dispatch(clusterCountX, clusterCountY, clusterCountZ);

			computeCommandList->BufferUAVToCopySource(activeClusterCount);
			computeCommandList->CopyBufferToBuffer(activeClusterCount, 0, readBackActiveClusterCount, 4, false);
			computeCommandList->BufferCopySourceToUAV(activeClusterCount);
		}

		{
			computeCommandList->End();
			computeCommandQueue->Submit({ computeCommandList });

			computeFence->Signal(computeCommandQueue.get());
			computeFence->BlockCPU();
		}

		{
			UINT nActiveClusters = 0;
			std::span<UINT> activeClusterCountSpan(&nActiveClusters, 1);
			readBackActiveClusterCount->ReadData<UINT>(activeClusterCountSpan);

			statistics.activeClusterCount = nActiveClusters;
			//std::cout << "Active Cluster Count: " << nActiveClusters << "/" << clusterCount << std::endl;
		}
	}

	void Renderer::ClusterAssignmentPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ClusterAssignmentPass");

		RHI::ComputePipeline::Ref pso = computePsoCache[RenderPassSemantic::ClusterAssignment];

		RHI::CommandList::Ref computeCommandList = rctx->GetComputeCommandList();
		RHI::CommandQueue::Ref computeCommandQueue = rctx->GetComputeQueue();
		RHI::Fence::Ref computeFence = rctx->GetComputeFence();

		RHI::Buffer::Ref activeClusterIndex = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Buffer::Ref clusterBuffer = frameGraph->GetInputBufferResource(pass, 1);
		RHI::Buffer::Ref uploadLightCompBuffer = frameGraph->GetInputBufferResource(pass, 2);
		RHI::Buffer::Ref constantBuffer = frameGraph->GetInputBufferResource(pass, 3);
		RHI::Buffer::Ref readBackActiveClusterCount = frameGraph->GetInputBufferResource(pass, 4);

		RHI::Buffer::Ref lightCompBuffer = frameGraph->GetOutputBufferResource(pass, 0);
		RHI::Buffer::Ref lightGridPtr = frameGraph->GetOutputBufferResource(pass, 1);
		RHI::Buffer::Ref clusterDataBuffer = frameGraph->GetOutputBufferResource(pass, 2);
		RHI::Buffer::Ref lightGridBuffer = frameGraph->GetOutputBufferResource(pass, 3);

		Wiley::LightComponent* lightCompStorage = _scene->GetComponentStorage<Wiley::LightComponent>();
		UINT lightCompCount = _scene->GetComponentReach<Wiley::LightComponent>();

		UINT activeClusterCountData = 0;
		{
			struct ConstantBuffer {
				uint32_t lightCount;
				DirectX::XMFLOAT4X4 view;
			}constantBufferData;

			constantBufferData.lightCount = lightCompCount;
			DirectX::XMStoreFloat4x4(&constantBufferData.view, viewMatrix);
			constantBuffer->UploadData(&constantBufferData, WILEY_SIZEOF(ConstantBuffer), 0, 0);

			uploadLightCompBuffer->UploadData<Wiley::LightComponent>(std::span<Wiley::LightComponent>(lightCompStorage, lightCompCount));
		}

		{
			std::span<UINT> activeClusterCountSpan(&activeClusterCountData, 1);
			readBackActiveClusterCount->ReadData<UINT>(activeClusterCountSpan);
		}

		{
			auto& heaps = rctx->GetDescriptorHeaps();
			computeCommandList->Begin({ heaps.cbv_srv_uav });
			computeCommandList->SetComputePipeline(pso);
			computeCommandList->SetComputeRootSignature(pso->GetRootSignature());
		}

		{
			computeCommandList->BindComputeShaderResource(constantBuffer->GetCBV(), 0);
			computeCommandList->BindComputeShaderResource(clusterBuffer->GetUAV(), 1);
			computeCommandList->BindComputeShaderResource(activeClusterIndex->GetUAV(), 2);
			computeCommandList->BindComputeShaderResource(lightCompBuffer->GetUAV(), 3);
			computeCommandList->BindComputeShaderResource(lightGridPtr->GetUAV(), 4);
			computeCommandList->BindComputeShaderResource(clusterDataBuffer->GetUAV(), 5);
			computeCommandList->BindComputeShaderResource(lightGridBuffer->GetUAV(), 6);
		}

		{
			computeCommandList->BufferUAVToCopyDest(lightCompBuffer);
			computeCommandList->CopyBufferToBuffer(uploadLightCompBuffer, 0, lightCompBuffer, WILEY_SIZEOF(Wiley::LightComponent) * lightCompCount, true);
			computeCommandList->BufferCopyDestToUAV(lightCompBuffer);

			computeCommandList->ClearUAVUint(lightGridPtr);
			computeCommandList->UAVBarrier(lightGridPtr);
			computeCommandList->Dispatch(activeClusterCountData, 1, 1);

		}

		{
			computeCommandList->End();
			computeCommandQueue->Submit({ computeCommandList });

			computeFence->Signal(computeCommandQueue.get());
			computeFence->BlockCPU();
		}
	}

	void Renderer::ClusterHeatMapPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ClusterHeatMapPass");

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::ClusterHeapMapPass];

		auto commandList = rctx->GetCurrentCommandList();
		auto& heaps = rctx->GetDescriptorHeaps();

		RHI::Buffer::Ref clusterDataBuffer = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Texture::Ref depthMap = frameGraph->GetInputTextureResource(pass, 1);
		RHI::Buffer::Ref constantBuffer = frameGraph->GetInputBufferResource(pass, 2);

		RHI::Texture::Ref clusterHeatMap = frameGraph->GetOutputTextureResource(pass, 0);

		uint32_t width = 0;
		uint32_t height = 0;

		{
			window->GetClientDimensions(width, height);
			commandList->SetViewport(static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.0f, 0.0f);

			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
		}


		const auto camera = _scene->GetCamera();

		UINT clusterCountX = (width + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountY = (height + TILE_GRID_SIZE - 1) / TILE_GRID_SIZE;
		UINT clusterCountZ = CLUSTER_DEPTH;
		WILEY_MAYBE_UNUSED UINT clusterCount = clusterCountX * clusterCountY * clusterCountZ;

		struct ConstantBuffer {
			DirectX::XMUINT4 clusterCount;

			DirectX::XMUINT2 tileSize;

			float nearPlane;
			float farPlane;
		}cBufferData;

		cBufferData.clusterCount = { clusterCountX, clusterCountY, clusterCountZ, 1 };
		cBufferData.nearPlane = camera->GetNear();
		cBufferData.farPlane = camera->GetFar();
		cBufferData.tileSize = { TILE_GRID_SIZE,TILE_GRID_SIZE };
	
		constantBuffer->UploadData<ConstantBuffer>(std::span(&cBufferData, 1));

		frameGraph->TransitionOutputTextures(pass);
		commandList->BufferUAVToAllShader(clusterDataBuffer);
		commandList->ImageBarrier(depthMap, RHI::TextureUsage::ShaderResource);
		
		commandList->SetRenderTargets({ clusterHeatMap->GetRTVDescriptor() }, RHI::DescriptorHeap::Descriptor::Invalid());
		commandList->ClearRenderTarget({ clusterHeatMap->GetRTVDescriptor() }, { 1.0f,1.0f,1.0f,1.0f });
		commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

		commandList->BindShaderResource(constantBuffer->GetCBV(), 0);
		commandList->BindShaderResource(depthMap->GetSRV(), 1);
		commandList->BindShaderResource(clusterDataBuffer->GetSRV(), 2);

		commandList->DrawInstanced(6, 1);

		commandList->BufferAllShaderToUAV(clusterDataBuffer);
		frameGraph->TransitionOutputTextureToCreationState(pass);
		commandList->ImageBarrier(depthMap, depthMap->GetBeforeState());

	}

}