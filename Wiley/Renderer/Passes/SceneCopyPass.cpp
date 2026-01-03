#include "../Renderer.h"

namespace Renderer3D {


	void Renderer::SceneCopyPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::SceneCopyPass");

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();

		if (!isVertexIndexDataDirty[graphicsRingIndex]) {
#ifdef _DEBUG
			//std::cout << "Skipping Vertex/Index Data Upload." << std::endl;
#endif
			return;
		}
		else {
			isVertexIndexDataDirty[graphicsRingIndex] = false;
		}

		auto __fence = rctx->GetGraphicsFence(graphicsRingIndex);
		__fence->BlockGPU(rctx->GetCopyCommandQueue().get());

		RHI::CommandList::Ref copyCommandList = rctx->GetCopyCurrentCommandList();
		copyCommandList->Begin({});

		std::shared_ptr<Wiley::ResourceCache> resourceCache = _scene->GetResourceCache();

		auto vertexPool = resourceCache->GetVertexPool();
		auto indexPool = resourceCache->GetIndexPool();
		auto mtlDataPool = resourceCache->GetMaterialDataPool();

		RHI::Buffer::Ref uploadMaterialData = frameGraph->GetOutputBufferResource(pass, 2);
		RHI::Buffer::Ref materialDataBuffer = frameGraph->GetOutputBufferResource(pass, 3);

		const auto& vertexUploadBuffer = resourceCache->GetVertexUploadBuffer();
		const auto& indexUploadBuffer = resourceCache->GetIndexUploadBuffer();

		//Copy Data into upload buffers.
		{
			UINT8* bufferPtr = nullptr;
			uploadMaterialData->Map(reinterpret_cast<void**>(&bufferPtr), 0, 0);
			uploadMaterialData->UploadPersistent(bufferPtr, mtlDataPool->GetBasePtr(), mtlDataPool->GetReach());
			uploadMaterialData->Unmap(0, 0);
		}

		//Copy Data to default buffers.
		{
			copyCommandList->CopyBufferToBuffer(vertexUploadBuffer, 0, vertexBuffer[graphicsRingIndex], vertexPool->GetReach());
			copyCommandList->CopyBufferToBuffer(indexUploadBuffer, 0, indexBuffer[graphicsRingIndex], indexPool->GetReach());
			copyCommandList->CopyBufferToBuffer(uploadMaterialData, 0, materialDataBuffer, mtlDataPool->GetReach());
		}

		copyCommandList->End();
		rctx->ExecuteCopyCommandList({ copyCommandList });

		RHI::Fence::Ref copyFence = rctx->GetCopyFence(graphicsRingIndex);

		copyFence->Signal(rctx->GetCopyCommandQueue().get());
		copyFence->BlockCPU();
	}

}