#include "Renderer.h"
#include "../Scene/Entity.h"

namespace Renderer3D {
	UINT fakeConstant = 5;

	void Renderer::WireframePass(RenderPass& pass)
	{
		OPTICK_EVENT(__FUNCTION__);

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::Wireframe];

		Wiley::cBuffer cBufferData;

		DirectX::XMMATRIX mvp = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
		DirectX::XMStoreFloat4x4(&cBufferData.vp, XMMatrixTranspose(mvp));

		memcpy(cPtr, &cBufferData, sizeof(Wiley::cBuffer));

		auto commandList = rctx->GetCurrentCommandList();

		commandList->SetGraphicsPipeline(pso);
		commandList->SetGraphicsRootSignature(pso->GetRootSignature());

		commandList->PushConstant(&fakeConstant, 4, 0);
		commandList->BindShaderResource(cBufferDesc, 1);

		commandList->SetViewport(pass.passDimensions.x, pass.passDimensions.y, 0, 0);

		RHI::DescriptorHeap::Descriptor dsv = depthResource->GetDSVDescriptor();

		std::vector<RHI::DescriptorHeap::Descriptor> rtv(pass.outputs.size());

		for (int i = 0; auto& handle : pass.outputs)
		{
			RHI::Texture::Ref renderTarget = std::get<RHI::Texture::Ref>(frameGraph->GetResource(handle).resource);
			rtv[i] = renderTarget->GetRTVDescriptor();
			i++;
		}

		commandList->ImageBarrier(depthResource, RHI::TextureUsage::DepthStencilTarget);
		frameGraph->TransitionOutputTextures(pass);

		commandList->SetRenderTargets(rtv, dsv);
		commandList->ClearRenderTarget(rtv, { 0.0f,0.0f,0.0f,0.0f });
		commandList->ClearDepthTarget(dsv);

		commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::LineList);
		//commandList->ExecuteIndirect(indirectCommandBuffer);

		commandList->ImageBarrier(depthResource, RHI::TextureUsage::Common);
	}

	void Renderer::ShadowPass(RenderPass& pass)
	{
		OPTICK_EVENT(__FUNCTION__);

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::ShadowPass];

		Wiley::cBufferLight cBufferData;
		DirectX::XMMATRIX vp = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(1.0f, 1.0f, -3.0f, 1.0),
			DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));

		DirectX::XMMATRIX mvp = DirectX::XMMatrixMultiply(vp,
			DirectX::XMMatrixOrthographicOffCenterLH(-30.0, 30.0f, -30.0f, 30.0f, -100.1f, 100.0f));
		DirectX::XMStoreFloat4x4(&cBufferData.vp, XMMatrixTranspose(mvp));

		memcpy(cLightPtr, &cBufferData, sizeof(Wiley::cBufferLight));

		auto commandList = rctx->GetCurrentCommandList();

		commandList->SetGraphicsPipeline(pso);
		commandList->SetGraphicsRootSignature(pso->GetRootSignature());

		commandList->PushConstant(&fakeConstant, 4, 0);
		commandList->BindShaderResource(cBufferLightDesc, 1);

		commandList->SetViewport(pass.passDimensions.x, pass.passDimensions.y, 0, 0);

		RHI::DescriptorHeap::Descriptor dsv = frameGraph->GetOutputTextureResource(pass, 0)->GetDSVDescriptor();;

		frameGraph->TransitionOutputTextures(pass);

		commandList->SetRenderTargets({}, dsv);
		commandList->ClearDepthTarget(dsv);
		commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

		//commandList->ExecuteIndirect(indirectCommandBuffer);
	}

	void Renderer::PresentPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::PresentPass");

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();
		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::PresentPass];

		Wiley::cBufferLight cBufferData;
		DirectX::XMMATRIX vp = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(1.0f, 1.0f, -3.0f, 1.0),
			DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));

		DirectX::XMMATRIX mvp = DirectX::XMMatrixMultiply(vp,
			DirectX::XMMatrixOrthographicOffCenterLH(-30.0, 30.0f, -30.0f, 30.0f, -100.1f, 100.0f));
		DirectX::XMStoreFloat4x4(&cBufferData.vp, XMMatrixTranspose(mvp));

		memcpy(cLightPtr, &cBufferData, sizeof(Wiley::cBufferLight));

		auto commandList = rctx->GetCurrentCommandList();

		commandList->SetGraphicsPipeline(pso);
		commandList->SetGraphicsRootSignature(pso->GetRootSignature());

		RHI::Buffer::Ref constantBuffer = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Texture::Ref positionDataMap = frameGraph->GetInputTextureResource(pass, 1);

		RHI::Texture::Ref offscreenTexture = frameGraph->GetOutputTextureResource(pass, 0);

		{
			const auto& env = _scene->GetEnvironment();

			struct ConstantBuffer {
				DirectX::XMFLOAT3 whitePoint;
				float exposure;
				float gamma;
			}cBufferData;

			cBufferData.exposure = env.exposure;
			cBufferData.whitePoint = env.whitePoint;
			cBufferData.gamma = env.gamma;

			constantBuffer->UploadData(&cBufferData, sizeof(ConstantBuffer), 0, 0);
		}

		
		frameGraph->TransistionInputTextures(pass);
		frameGraph->TransitionOutputTextures(pass);

		commandList->BindShaderResource(constantBuffer->GetCBV(), 0);
		commandList->BindShaderResource(positionDataMap->GetSRV(), 1);
		commandList->BindSamplerResource(sampler->GetDescriptor(), 2);

		commandList->SetViewport(viewportWidth, viewportHeight, 0, 0);

		auto rtv = offscreenTexture->GetRTVDescriptor();
		commandList->SetRenderTargets({ rtv }, RHI::DescriptorHeap::Descriptor::Invalid());
		commandList->ClearRenderTarget({ rtv }, { 1.0f,1.0f,1.0f,1.0f });

		commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

		commandList->DrawInstanced(6, 1);

		outputTexture = offscreenTexture;

		frameGraph->TransitionInputTextureToCreationState(pass);	


	}	
}