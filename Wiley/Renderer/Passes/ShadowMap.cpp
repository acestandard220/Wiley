#include "../Renderer.h"
#include "../Scene/Entity.h"


namespace Renderer3D {


	void ExecutePointLight(RHI::CommandList::Ref& commandList, const Wiley::LightComponent& light, ShadowMapManager::Ref shadowMapManager, Wiley::Camera::Ref camera, std::vector<DrawCommand>drawCommandCache)
	{
		ZoneScopedN("Renderer::ShadowMapPass->Execute");

		const auto depthMap = shadowMapManager->GetDepthMap(light.depthMapIndex);
		const auto& depthMapRTVs = depthMap->GetDepthMapRTVs();
		const auto mapSize = depthMap->GetWidth();

		const auto& mainDepthBuffer = shadowMapManager->GetDummyDepthTexture(static_cast<Renderer3D::ShadowMapSize>(mapSize));

		{
			commandList->SetViewport(mapSize, mapSize, 0, 0);
			commandList->ImageBarrier(depthMap, RHI::TextureUsage::RenderTarget);
		}

		for (int f = 0; f < 6; f++)
		{
			{
				commandList->SetRenderTargets({ depthMapRTVs[f] }, mainDepthBuffer->GetDSVDescriptor());
				commandList->ClearRenderTarget({ depthMapRTVs[f] }, { 1.0f,1.0f,1.0f,1.0f });
				commandList->ClearDepthTarget(mainDepthBuffer->GetDSVDescriptor());
			}

			struct PushConstants {
				uint32_t drawID;
				uint32_t vpIndex;
				float farPlane;
				uint32_t _pad1;

				DirectX::XMFLOAT3 lightPosition;
				uint32_t _pad2;
			}pConstants;

			pConstants.farPlane = camera->GetFar();
			pConstants.vpIndex = light.matrixIndex + f;
			pConstants.lightPosition = light.position;

			for (int i = 0; i < drawCommandCache.size(); i++) {
				const DrawCommand& drawCmd = drawCommandCache[i];

				pConstants.drawID = drawCmd.drawID;

				commandList->PushConstant(&pConstants, 8 * 4, 0);
				commandList->DrawInstancedIndexed(drawCmd.indexCount, drawCmd.instanceCount,
					drawCmd.indexStartLocation, drawCmd.vertexStartLocation, drawCmd.instanceStartIndex);
			}
		}		
	}

	void Renderer3D::Renderer::ShadowMapPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ShadowMapPass");

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::PointShadowMapPass];

		RHI::Buffer::Ref meshFilterBuffer = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Buffer::Ref subMeshDataBuffer = frameGraph->GetInputBufferResource(pass, 1);
		RHI::Buffer::Ref meshInstanceIndex = frameGraph->GetInputBufferResource(pass, 2);
		RHI::Buffer::Ref meshInstanceBaseBuffer = frameGraph->GetInputBufferResource(pass, 3);

		RHI::Buffer::Ref uploadLightViewProjections = frameGraph->GetInputBufferResource(pass, 4);
		RHI::Buffer::Ref lightViewProjections = frameGraph->GetInputBufferResource(pass, 5);

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();
		auto commandList = rctx->GetCurrentCommandList();

		const auto& heaps = rctx->GetDescriptorHeaps();
		const auto shadowMapManager = _scene->GetShadowMapManager();

		{
			commandList->BufferUAVToNonPixelShader({
				meshFilterBuffer
			});
		}

		if (shadowMapManager->GetDirtyEntities().size() || shadowMapManager->GetDirtyPointLight().size() || shadowMapManager->IsAllLightEntityDirty())
		{
			const auto span = shadowMapManager->GetViewProjectionsHead();
			uploadLightViewProjections->UploadData<DirectX::XMFLOAT4X4>(span);

			commandList->BufferBarrier(lightViewProjections, D3D12_RESOURCE_STATE_COPY_DEST);
			commandList->CopyBufferToBuffer(uploadLightViewProjections, 0, lightViewProjections, span.size_bytes(), false);
			commandList->BufferBarrier(lightViewProjections, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		{
			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
			commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

			commandList->BindShaderResource(meshFilterBuffer->GetSRV(), 1);
			commandList->BindShaderResource(subMeshDataBuffer->GetSRV(), 2);
			commandList->BindShaderResource(meshInstanceBaseBuffer->GetSRV(), 3);
			commandList->BindShaderResource(meshInstanceIndex->GetSRV(), 4);
			commandList->BindShaderResource(lightViewProjections->GetSRV(), 5);
		}


		std::vector<RHI::Barrier> depthToPixelBarriers;

		if(shadowMapManager->IsAllLightEntityDirty())
		{
			for (auto& lightEntt : _scene->GetEntitiesWith<Wiley::LightComponent>()) {

				auto& light = lightEntt.GetComponent<Wiley::LightComponent>();
				switch (light.type) {
					case Wiley::LightType::Point: {
						ExecutePointLight(commandList, light, shadowMapManager, camera, drawCommandCache);
						break;
					}
					case Wiley::LightType::Directional: {

						break;
					}
					case Wiley::LightType::Spot: {

						break;
					}
				}
				depthToPixelBarriers.push_back({ shadowMapManager->GetDepthMap(light.depthMapIndex),RHI::TextureUsage::PixelShaderResource });
			}
			shadowMapManager->CleanAllLightEntity();
		}



		auto dirtyPointLights = shadowMapManager->GetDirtyPointLight();
		while (dirtyPointLights.size())
		{
			Wiley::Entity lightEntt = Wiley::Entity(dirtyPointLights.front(), _scene.get());
			const auto& light = lightEntt.GetComponent<Wiley::LightComponent>();

			ExecutePointLight(commandList, light, shadowMapManager, camera, drawCommandCache);

			depthToPixelBarriers.push_back({ shadowMapManager->GetDepthMap(light.depthMapIndex),RHI::TextureUsage::PixelShaderResource });
			dirtyPointLights.pop();
		}
		shadowMapManager->ClearDirtyPointLightQueue();




		{
			commandList->ImageBarrier(depthToPixelBarriers);
			commandList->BufferNonPixelShaderToUAV({
				meshFilterBuffer
			});
		}


		{
			commandList->End();
			rctx->ExecuteGraphicsCommandList({ commandList });
		}

		{
			RHI::Fence::Ref gfxFence = rctx->GetCurrentGraphicsFence();
			RHI::CommandQueue::Ref commandQueue = rctx->GetCommandQueue();
			gfxFence->Signal(commandQueue.get());
			gfxFence->BlockCPU();
		}

		{
			commandList->Begin({ heaps.cbv_srv_uav,heaps.sampler });
			commandList->BindVertexBuffer(vertexBuffer[graphicsRingIndex]);
			commandList->BindIndexBuffer(indexBuffer[graphicsRingIndex]);
		}
	}


}