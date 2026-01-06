#include "../Renderer.h"

namespace Renderer3D {


	void Renderer::LightingPass(RenderPass& pass) {
		ZoneScopedN("Renderer::LightingPass");

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::LightingPass];

		auto commandList = rctx->GetCurrentCommandList();

		{
			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
		}

		RHI::Texture::Ref positionDataMap = frameGraph->GetInputTextureResource(pass, 0);
		RHI::Texture::Ref normalDataMap = frameGraph->GetInputTextureResource(pass, 1);
		RHI::Texture::Ref colorDataMap = frameGraph->GetInputTextureResource(pass, 2);
		RHI::Texture::Ref armDataMap = frameGraph->GetInputTextureResource(pass, 3);

		RHI::Buffer::Ref lightCompBuffer = frameGraph->GetInputBufferResource(pass, 4);


		RHI::Texture::Ref lightPassMap = frameGraph->GetOutputTextureResource(pass, 0);

		const auto& cameraPosition = _scene->GetCamera()->GetPosition();

		const auto& env = _scene->GetEnvironment();
		const auto& em = env.currentEnvirontmentMap;
		const bool doIBL = env.doIBL;

		const UINT lightCompCount = _scene->GetComponentReach<Wiley::LightComponent>();


		{
			frameGraph->TransistionInputTextures(pass);
			frameGraph->TransitionOutputTextures(pass);

			commandList->BufferUAVToPixelShader({
				lightCompBuffer
			});
		}

		{
			commandList->BindShaderResource(positionDataMap->GetSRV(), 0);
			commandList->BindShaderResource(normalDataMap->GetSRV(), 1);
			commandList->BindShaderResource(colorDataMap ->GetSRV(), 2);
			commandList->BindShaderResource(armDataMap->GetSRV(), 3);
			commandList->BindSamplerResource(gBufferReadSampler->GetDescriptor(), 4);

			commandList->BindShaderResource(em->irradianceMap->GetSRV(), 5);

			struct ConstantPush { 
				DirectX::XMFLOAT4 cameraPosition;
				uint32_t doIBL;
				uint32_t lightComCount;
			}cp;

			cp.cameraPosition = cameraPosition;
			cp.doIBL = doIBL;
			cp.lightComCount = lightCompCount;

			commandList->PushConstant(&cp, sizeof(ConstantPush), 6);
			commandList->BindShaderResource(lightCompBuffer->GetSRV(), 7);

			const auto smm = _scene->GetShadowMapManager();
			commandList->BindShaderResource(smm->GetCubeSRVHead(), 8);
			commandList->BindShaderResource(smm->GetArraySRVHead(), 9);

			commandList->BindShaderResource(postProcessSampler->GetDescriptor(), 11);

			commandList->BindShaderResource(em->prefilteredMap->GetSRV(), 12);
			commandList->BindShaderResource(em->brdfLUT->GetSRV(), 13);
		}
		
		{
			commandList->SetViewport(viewportWidth, viewportHeight, 0, 0);

			auto rtv = lightPassMap->GetRTVDescriptor();
			commandList->SetRenderTargets({ rtv }, RHI::DescriptorHeap::Descriptor::Invalid());
			commandList->ClearRenderTarget({ rtv }, { 1.0,1.0f,1.0f,1.0f });
			commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

		}

		{
			commandList->DrawInstanced(6, 1);
		}


		{
			frameGraph->TransitionInputTextureToCreationState(pass);
			frameGraph->TransitionOutputTextureToCreationState(pass);
			commandList->BufferPixelShaderToUAV({
				lightCompBuffer
			});
		}


	}

	void Renderer::SkyboxPass(RenderPass& pass)
	{
		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::SkyboxPass];

		RHI::Buffer::Ref constantBuffer = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Texture::Ref depthMap = frameGraph->GetInputTextureResource(pass, 1);
		RHI::Texture::Ref lightPassMap = frameGraph->GetInputTextureResource(pass, 2);


		auto commandList = rctx->GetCurrentCommandList();

		{
			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
		}

		{
			frameGraph->TransistionInputTextures(pass);
		}
		const auto& env = _scene->GetEnvironment();
		const auto& em = env.currentEnvirontmentMap;

		const auto bgColor = env.backgroundColor;
		const auto doIBL = env.doIBL;

		struct ConstantBuffer {
			DirectX::XMFLOAT4X4 view;
			DirectX::XMFLOAT4X4 projection;

			DirectX::XMFLOAT4 bgColor;
		}; 
		ConstantBuffer cBufferData{};

		DirectX::XMStoreFloat4x4(&cBufferData.view, viewMatrix);
		DirectX::XMStoreFloat4x4(&cBufferData.projection, projectionMatrix);

		cBufferData.view._14 = 0.0f;
		cBufferData.view._24 = 0.0f;
		cBufferData.view._34 = 0.0f;

		cBufferData.bgColor = { env.backgroundColor.x,env.backgroundColor.y,env.backgroundColor.z,static_cast<float>(doIBL) };
		
		constantBuffer->UploadData(&cBufferData, sizeof(ConstantBuffer), 0, 0);

		{
			commandList->PushConstant(&cBufferData, 36 * 4, 0);
			commandList->BindShaderResource(em->cubeMap->GetSRV(), 1);
			commandList->BindSamplerResource(postProcessSampler->GetDescriptor(), 2);
		}

		{
			commandList->SetViewport(viewportWidth, viewportHeight, 0, 0);

			auto rtv = lightPassMap->GetRTVDescriptor();
			commandList->SetRenderTargets({ rtv }, depthMap->GetDSVDescriptor());			

			commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);
		}

		{
			commandList->DrawInstanced(36, 1);
		}

		{
			frameGraph->TransitionInputTextureToCreationState(pass);
		}
	}

}