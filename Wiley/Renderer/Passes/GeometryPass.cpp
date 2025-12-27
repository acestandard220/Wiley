#include "../Renderer.h"

namespace Renderer3D {

	void Renderer::GeometryPass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::GeometryPass");

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::Geometry];

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();

		Wiley::cBuffer cBufferData{};
		DirectX::XMStoreFloat4x4(&cBufferData.vp, viewProjection);

		memcpy(cPtr, &cBufferData, sizeof(Wiley::cBuffer));

		auto commandList = rctx->GetCurrentCommandList();

		{
			commandList->SetViewport(viewportWidth, viewportHeight, 0, 0);

			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
		}

		std::vector<RHI::DescriptorHeap::Descriptor> rtv(4);

		for (int i = 0; i < 4; i++)
		{
			RHI::Texture::Ref renderTarget = std::get<RHI::Texture::Ref>(frameGraph->GetResource(pass.outputs[i]).resource);
			rtv[i] = renderTarget->GetRTVDescriptor();
		}

		RHI::Buffer::Ref meshFilterBuffer = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Buffer::Ref subMeshDataBuffer = frameGraph->GetInputBufferResource(pass, 1);
		RHI::Buffer::Ref mtlDataBuffer = frameGraph->GetInputBufferResource(pass, 2);

		RHI::Buffer::Ref meshInstanceIndexBuffer = frameGraph->GetInputBufferResource(pass, 3);
		RHI::Buffer::Ref meshInstanceBaseBuffer = frameGraph->GetInputBufferResource(pass, 4);

		RHI::Texture::Ref depthBuffer = frameGraph->GetOutputTextureResource(pass, 4);

		commandList->BufferBarrier(meshFilterBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		frameGraph->TransitionOutputTextures(pass);

		{
			commandList->BindShaderResource(cBufferDesc, 1);
			commandList->BindShaderResource(meshFilterBuffer->GetSRV(), 2);
			commandList->BindShaderResource(subMeshDataBuffer->GetSRV(), 3);
			commandList->BindShaderResource(mtlDataBuffer->GetSRV(), 4);
			commandList->BindShaderResource(pbrSampler->GetDescriptor(), 5);
			commandList->BindShaderResource(_scene->GetResourceCache()->GetImageTextureDescriptorStart(Wiley::MapType::Albedo), 6);
			commandList->BindShaderResource(_scene->GetResourceCache()->GetImageTextureDescriptorStart(Wiley::MapType::Normal), 7);
			commandList->BindShaderResource(_scene->GetResourceCache()->GetImageTextureDescriptorStart(Wiley::MapType::Metalloic), 8);
			commandList->BindShaderResource(meshInstanceBaseBuffer->GetSRV(), 9);
			commandList->BindShaderResource(meshInstanceIndexBuffer->GetSRV(), 10);

		}

		{
			commandList->SetRenderTargets(rtv, depthBuffer->GetDSVDescriptor());
			commandList->ClearRenderTarget(rtv, { 1.0f,1.0f,1.0f,1.0f });
			commandList->ClearDepthTarget(depthBuffer->GetDSVDescriptor());
			commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);
		}

		{
			ZoneScopedN("GeometryPassDrawCmdExec.");

			for (int i = 0; i < drawCommandCache.size(); i++) {
				const DrawCommand& drawCmd = drawCommandCache[i];

				commandList->PushConstant(&drawCmd.drawID, 4, 0);
				commandList->DrawInstancedIndexed(drawCmd.indexCount, drawCmd.instanceCount,
					drawCmd.indexStartLocation, drawCmd.vertexStartLocation, drawCmd.instanceStartIndex);
			}
		}

		{
			commandList->BufferBarrier(meshFilterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			frameGraph->TransitionOutputTextureToCreationState(pass);
		}
	}

}