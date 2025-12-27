#include "../Renderer.h"

namespace Renderer3D {

	void Renderer::DepthPrePass(RenderPass& pass)
	{
		ZoneScopedN("Renderer::DepthPrePass");

		RHI::GraphicsPipeline::Ref pso = gfxPsoCache[RenderPassSemantic::DepthPrepass];

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();
		auto commandList = rctx->GetCurrentCommandList();
		auto& heaps = rctx->GetDescriptorHeaps();

		//Start new List Record for this pass since some compute pass depend on its completion.
		{
			commandList->Begin({ heaps.cbv_srv_uav,heaps.sampler });
		}

		{
			commandList->BindVertexBuffer(vertexBuffer[graphicsRingIndex]);
			commandList->BindIndexBuffer(indexBuffer[graphicsRingIndex]);
			
			commandList->SetGraphicsPipeline(pso);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
		}

		RHI::Buffer::Ref meshFilterBuffer = frameGraph->GetInputBufferResource(pass, 0);
		RHI::Buffer::Ref submeshData = frameGraph->GetInputBufferResource(pass, 1);
		RHI::Buffer::Ref prePassCbuffer = frameGraph->GetInputBufferResource(pass, 2);
		RHI::Buffer::Ref meshInstanceIndexBuffer = frameGraph->GetInputBufferResource(pass, 3);
		RHI::Buffer::Ref meshInstanceBaseBuffer = frameGraph->GetInputBufferResource(pass, 4);


		RHI::Texture::Ref depthBuffer = frameGraph->GetOutputTextureResource(pass, 0);

		{
			struct _Cbuffer {
				DirectX::XMFLOAT4X4 vp;
			}constants;
			DirectX::XMStoreFloat4x4(&constants.vp, viewProjection);
			prePassCbuffer->UploadData(&constants, WILEY_SIZEOF(_Cbuffer), 0, 0);
		}

		{
			commandList->ImageBarrier(depthBuffer, RHI::TextureUsage::DepthStencilTarget);
			commandList->BufferUAVToNonPixelShader({
				meshFilterBuffer,meshInstanceIndexBuffer,meshInstanceBaseBuffer
			});
		}

		{
			commandList->BindShaderResource(prePassCbuffer->GetCBV(), 1);
			commandList->BindShaderResource(meshFilterBuffer->GetSRV(), 2);
			commandList->BindShaderResource(submeshData->GetSRV(), 3);

			commandList->BindShaderResource(meshInstanceBaseBuffer->GetSRV(), 4);
			commandList->BindShaderResource(meshInstanceIndexBuffer->GetSRV(), 5);
		}

		{
			commandList->SetViewport(viewportWidth, viewportHeight, 0, 0);

			commandList->SetRenderTargets({}, depthBuffer->GetDSVDescriptor());
			commandList->ClearDepthTarget(depthBuffer->GetDSVDescriptor());
			commandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);
		}

		{
			ZoneScopedN("DepthPrepassDrawCmdExec.");

			for (int i = 0; i < drawCommandCache.size(); i++) {
				const DrawCommand& drawCmd = drawCommandCache[i];

				commandList->PushConstant(&drawCmd.drawID, 4, 0);
				commandList->DrawInstancedIndexed(drawCmd.indexCount, drawCmd.instanceCount,
					drawCmd.indexStartLocation, drawCmd.vertexStartLocation, drawCmd.instanceStartIndex);
			}
		}

		{
			commandList->ImageBarrier(depthBuffer, RHI::TextureUsage::NonPixelShader);

			commandList->BufferNonPixelShaderToUAV({
				meshFilterBuffer,meshInstanceIndexBuffer,meshInstanceBaseBuffer
			});
		}

		//Close Graphics List & Begin new one for preceeding passes.
		{
			ZoneScopedN("DepthPrepassNewCommandBufferRecording");

			{
				ZoneScopedN("EndDepthPrepassRec");

				commandList->End();
				rctx->ExecuteGraphicsCommandList({ commandList });
			}

			//Wait till the GPU finishes this early execute before clearing the allocator for the next ones.
			{
				ZoneScopedN("DepthPrepassFenceSignal");

				RHI::Fence::Ref gfxFence = rctx->GetCurrentGraphicsFence();
				RHI::CommandQueue::Ref commandQueue = rctx->GetCommandQueue();
				gfxFence->Signal(commandQueue.get());
				gfxFence->BlockCPU();

				pass.waitValue = gfxFence->GetWaitValue();
			}

			//Start New Record for following passes.
			//Passes that follow may choose to close the command list early and submit or keep it open to record.
			{
				ZoneScopedN("PostDepthPrepassBeginNewCmdListRec");

				commandList->Begin({ heaps.cbv_srv_uav,heaps.sampler });
				commandList->BindVertexBuffer(vertexBuffer[graphicsRingIndex]);
				commandList->BindIndexBuffer(indexBuffer[graphicsRingIndex]);
			}
		}

	}
}