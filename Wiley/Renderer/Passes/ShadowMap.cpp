#include "../Renderer.h"
#include "../Scene/Entity.h"


namespace Renderer3D {


	void Execute(RHI::CommandList::Ref& commandList, const Wiley::LightComponent& light, Renderer* renderer, ShadowMapManager::Ref shadowMapManager) {

		ZoneScopedN("Renderer::ShadowMapPass->Execute");

		RHI::Texture::Ref depthBuffer = shadowMapManager->GetDepthMap(light.depthMapIndex);
		const auto depthMapWidth = depthBuffer->GetWidth();
		const auto depthMapHeight = depthBuffer->GetHeight();

		commandList->SetViewport(depthMapWidth, depthMapHeight, 0, 0);

		const auto* lightMatrixHead = shadowMapManager->GetLightProjection(light.matrixIndex);

		if (light.type == Wiley::LightType::Point)
		{
			for (int i = 0; i < 6; i++)
			{
				struct ConstantBuffer {
					DirectX::XMFLOAT4X4 vp;
				}cBufferData;

				cBufferData.vp = *(lightMatrixHead + i);

				const auto& dsv = depthBuffer->GetDepthMapDSV(i);
				commandList->SetRenderTargets({}, dsv);
				commandList->ClearDepthTarget(dsv);

				commandList->PushConstant(&cBufferData, WILEY_SIZEOF(ConstantBuffer), 0);

				renderer->DrawCommands(commandList);
			}
		}
		else if (light.type == Wiley::LightType::Directional) {

			for (int i = 0; i < 4; i++) {
				struct ConstantBuffer {
					DirectX::XMFLOAT4X4 vp;
				}cBufferData;

				cBufferData.vp = *(lightMatrixHead + i);

				const auto& dsv = depthBuffer->GetDepthMapDSV(i);
				commandList->SetRenderTargets({}, dsv);
				commandList->ClearDepthTarget(dsv);

				commandList->PushConstant(&cBufferData, WILEY_SIZEOF(ConstantBuffer), 0);

				renderer->DrawCommands(commandList);
			}
		}
		else if (light.type == Wiley::LightType::Spot) {
			struct ConstantBuffer {
				DirectX::XMFLOAT4X4 vp;
			}cBufferData;

			cBufferData.vp = *(lightMatrixHead);

			const auto& dsv = depthBuffer->GetDepthMapDSV(0);
			commandList->SetRenderTargets({}, dsv);
			commandList->ClearDepthTarget(dsv);

			commandList->PushConstant(&cBufferData, WILEY_SIZEOF(ConstantBuffer), 0);

			renderer->DrawCommands(commandList);
		}
	}

	void Renderer3D::Renderer::ShadowMapPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::ShadowMapPass");

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::ShadowMapPass];

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();
		auto commandList = rctx->GetCurrentCommandList();

		const auto& heaps = rctx->GetDescriptorHeaps();

		{
			commandList->Begin({ heaps.cbv_srv_uav,heaps.sampler });
		}

		{
			commandList->BindVertexBuffer(vertexBuffer[graphicsRingIndex]);
			commandList->BindIndexBuffer(indexBuffer[graphicsRingIndex]);

			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
		}

		{
			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
			commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);
		}

		const auto shadowMapManager = _scene->GetShadowMapManager();

		auto& entities = shadowMapManager->GetDirtyEntities();

		for (int i = 0; i < entities.size(); i++) {
			Wiley::Entity lightEntt = Wiley::Entity(entities.front(), _scene.get());
			const auto& light = lightEntt.GetComponent<Wiley::LightComponent>();
			Execute(commandList, light, this, shadowMapManager);
		}

		shadowMapManager->ClearnDirtyQueue();
		

		if (shadowMapManager->IsAllLightEntitiesDirty())
		{
			Wiley::LightComponent* lightComponent = _scene->GetComponentStorage<Wiley::LightComponent>();
			uint32_t lightComponentCount = _scene->GetComponentReach< Wiley::LightComponent>();

			std::span<Wiley::LightComponent>lightComponentSpan(lightComponent, lightComponentCount);

			for (const auto& light : lightComponentSpan)
			{
				Execute(commandList, light, this, shadowMapManager);
			}
			shadowMapManager->MakeAllLightClean();
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
	}


}