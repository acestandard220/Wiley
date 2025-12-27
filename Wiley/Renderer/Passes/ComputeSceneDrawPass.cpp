#include "../Renderer.h"

namespace Renderer3D {

	void Renderer::ComputeSceneDrawPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ComputeSceneDrawPass");

		RHI::CommandList::Ref computeCommandList = rctx->GetComputeCommandList();
		RHI::CommandQueue::Ref computeCommandQueue = rctx->GetComputeQueue();
		RHI::Fence::Ref computeFence = rctx->GetComputeFence();
		rctx->GetCurrentGraphicsFence()->BlockGPU(computeCommandQueue.get());

		computeCommandList->Begin({ rctx->GetDescriptorHeaps().cbv_srv_uav });

		Wiley::MeshFilterComponent* meshFilterCompPtr = _scene->GetComponentStorage<Wiley::MeshFilterComponent>();
		UINT meshFilterCompReach = _scene->GetComponentReach<Wiley::MeshFilterComponent>();

		//Get Render Pass Resources...
		RHI::Buffer::Ref _meshFilterBufferUp = frameGraph->GetOutputBufferResource(pass, 0);
		RHI::Buffer::Ref _meshFilterBuffer = frameGraph->GetOutputBufferResource(pass, 1);

		RHI::Buffer::Ref _uploadSubmeshData = frameGraph->GetOutputBufferResource(pass, 2);
		RHI::Buffer::Ref subMeshDataBuffer = frameGraph->GetOutputBufferResource(pass, 3);

		RHI::Buffer::Ref meshFilterIndexPtr = frameGraph->GetOutputBufferResource(pass, 4);
		RHI::Buffer::Ref readBackMeshCountBuffer = frameGraph->GetOutputBufferResource(pass, 5);

		RHI::Buffer::Ref uploadMeshInstanceBase = frameGraph->GetOutputBufferResource(pass, 6);
		RHI::Buffer::Ref meshInstanceBase = frameGraph->GetOutputBufferResource(pass, 7);

		RHI::Buffer::Ref uploadMeshFilterIndexBufferPreOcc = frameGraph->GetOutputBufferResource(pass, 8);
		RHI::Buffer::Ref meshFilterIndexBufferPreOcc = frameGraph->GetOutputBufferResource(pass, 9);

		RHI::Buffer::Ref meshFilterIndexBufferPostOcc = frameGraph->GetOutputBufferResource(pass, 10);

		RHI::Buffer::Ref readBackMeshFilterIndexBufferPostOcc = frameGraph->GetOutputBufferResource(pass, 11);
		RHI::Buffer::Ref readBackMeshInstanceBase = frameGraph->GetOutputBufferResource(pass, 12);


		std::shared_ptr<Wiley::ResourceCache> resourceCache = _scene->GetResourceCache();

		std::vector<uint32_t> meshFilterIndexes;
		std::vector<Wiley::MeshInstanceBase> meshInstanceBaseData;
		auto meshResources = resourceCache->GetResourceOfType<Wiley::Mesh>(Wiley::ResourceType::Mesh);


		{
			ZoneScopedN("MeshInstanceBaseSetup");

			for (const auto& meshResource : meshResources) {
				Wiley::MeshInstanceBase instanceBase{
					.offset = static_cast<uint32_t>(meshFilterIndexes.size()),
					.size = static_cast<uint32_t>(meshResource->instanceMeshFilterIndex.size())
				};
				meshInstanceBaseData.emplace_back(instanceBase);

				meshFilterIndexes.insert(
					meshFilterIndexes.end(),
					meshResource->instanceMeshFilterIndex.begin(),
					meshResource->instanceMeshFilterIndex.end()
				);
			}
		}

		std::span meshInstanceBaseDataSpan(meshInstanceBaseData);
		{
			uploadMeshInstanceBase->UploadData<Wiley::MeshInstanceBase>(meshInstanceBaseDataSpan);
			computeCommandList->BufferUAVToCopyDest(meshInstanceBase);
			computeCommandList->CopyBufferToBuffer(uploadMeshInstanceBase, 0, meshInstanceBase, meshInstanceBaseDataSpan.size_bytes(), true);
			computeCommandList->BufferCopyDestToUAV(meshInstanceBase);
		}

		std::span meshFilterIndexDataSpan(meshFilterIndexes);
		{
			uploadMeshFilterIndexBufferPreOcc->UploadData<uint32_t>(meshFilterIndexDataSpan);
			computeCommandList->BufferUAVToCopyDest(meshFilterIndexBufferPreOcc);
			computeCommandList->CopyBufferToBuffer(uploadMeshFilterIndexBufferPreOcc, 0, meshFilterIndexBufferPreOcc, meshFilterIndexDataSpan.size_bytes(), false);
			computeCommandList->BufferCopyDestToUAV(meshFilterIndexBufferPreOcc);
		}


		UINT8* meshFilterBufferPtr = nullptr;
		{
			_meshFilterBufferUp->Map(reinterpret_cast<void**>(&meshFilterBufferPtr), 0, 0);
			_meshFilterBufferUp->UploadPersistent(meshFilterBufferPtr, meshFilterCompPtr, sizeof(Wiley::MeshFilterComponent) * meshFilterCompReach);
			_meshFilterBufferUp->Unmap(0, 0);
		}

		UINT subMeshDataBufferSize = static_cast<UINT>((UINT8*)_scene->GetSubMeshDataTop() - (UINT8*)_scene->GetSubMeshDataHead());

		{
			UINT* subMeshDataBufferPtr = nullptr;
			_uploadSubmeshData->Map(reinterpret_cast<void**>(&subMeshDataBufferPtr), 0, 0);
			_uploadSubmeshData->UploadPersistent(subMeshDataBufferPtr, _scene->GetSubMeshDataHead(), subMeshDataBufferSize);
			_uploadSubmeshData->Unmap(0, 0);

			computeCommandList->CopyBufferToBuffer(_meshFilterBufferUp, 0, _meshFilterBuffer, sizeof(Wiley::MeshFilterComponent) * meshFilterCompReach);
			computeCommandList->CopyBufferToBuffer(_uploadSubmeshData, 0, subMeshDataBuffer, subMeshDataBufferSize);
		}


		{
			computeCommandList->SetComputePipeline(computePso);
			computeCommandList->SetComputeRootSignature(computePso->GetRootSignature());
		}

		DirectX::XMUINT3 groupCount = { static_cast<uint32_t>(meshInstanceBaseData.size()),1, 1 };
		{
			computeCommandList->PushComputeConstant(&groupCount, 12, 0);
			computeCommandList->BindComputeShaderResource(meshFilterIndexPtr->GetUAV(), 1);
			computeCommandList->BindComputeShaderResource(_meshFilterBuffer->GetUAV(), 2);
			computeCommandList->BindComputeShaderResource(meshInstanceBase->GetUAV(), 3);

			computeCommandList->BindComputeShaderResource(meshFilterIndexBufferPreOcc->GetUAV(), 4);
			computeCommandList->BindComputeShaderResource(meshFilterIndexBufferPostOcc->GetUAV(), 5);
		}

		{
			computeCommandList->ClearUAVUint(meshFilterIndexPtr);
			computeCommandList->Dispatch(groupCount.x, groupCount.y, groupCount.z);

			computeCommandList->UAVBarrier(meshFilterIndexPtr);
			computeCommandList->UAVBarrier(meshFilterIndexBufferPreOcc);
		}

		{
			computeCommandList->BufferUAVToCopySource(meshFilterIndexPtr);
			computeCommandList->CopyBufferToBuffer(meshFilterIndexPtr, 0, readBackMeshCountBuffer, WILEY_SIZEOF(UINT), false);
			computeCommandList->BufferCopySourceToUAV(meshFilterIndexPtr);

			computeCommandList->BufferUAVToCopySource(meshFilterIndexBufferPostOcc);
			computeCommandList->CopyBufferToBuffer(meshFilterIndexBufferPostOcc, 0, readBackMeshFilterIndexBufferPostOcc, meshFilterIndexBufferPostOcc->GetSize(), false);
			computeCommandList->BufferCopySourceToUAV(meshFilterIndexBufferPostOcc);

			computeCommandList->BufferUAVToCopySource(meshInstanceBase);
			computeCommandList->CopyBufferToBuffer(meshInstanceBase, 0, readBackMeshInstanceBase, meshInstanceBase->GetSize(), false);
			computeCommandList->BufferCopySourceToUAV(meshInstanceBase);
		}

		computeCommandList->End();
		computeCommandQueue->Submit({ computeCommandList });

		computeFence->Signal(computeCommandQueue.get());
		computeFence->BlockCPU();

		//Create draw commands
		{
			ZoneScopedN("CreateDrawCommands");

			Wiley::MeshInstanceBase* occMeshInstanceBufferPtr = nullptr;
			readBackMeshInstanceBase->Map(reinterpret_cast<void**>(&occMeshInstanceBufferPtr), 0, 0);

			drawCommandCache.clear();
			drawCommandCache.resize(meshInstanceBaseData.size());

			auto vertexPoolBase = resourceCache->GetVertexPoolBasePtr();
			auto indexPoolBase = resourceCache->GetIndexPoolBasePtr();

			for (int i = 0; i < meshInstanceBaseData.size(); i++) {
				Wiley::Mesh* _meshres = meshResources[i].get();

				WILEY_MUSTBE_UINTSIZE(_meshres->instanceMeshFilterIndex.size());

				UINT vertexStartLocation = static_cast<UINT>(_meshres->vertexBlock.data() - vertexPoolBase);
				UINT indexStartLocation = static_cast<UINT>(_meshres->indexBlock.data() - indexPoolBase);

				DrawCommand* drawCmd = &drawCommandCache[i];
				drawCmd->drawID = i;
				drawCmd->indexCount = _meshres->indexCount;
				drawCmd->indexStartLocation = indexStartLocation;
				drawCmd->instanceCount = _meshres->instanceMeshFilterIndex.size();
				drawCmd->vertexStartLocation = vertexStartLocation;
				drawCmd->instanceStartIndex = 0;
			}

			readBackMeshInstanceBase->Unmap(0, 0);
		}
	}
}