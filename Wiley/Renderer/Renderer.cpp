#include "Renderer.h"
#include "../Scene/Entity.h"

#include <thread>

namespace Renderer3D
{
	void Renderer::CompileFrameGraph()
	{
		ZoneScopedN("Renderer::CompileFrameGraph");

		//Importing needed external resources from C++ so they can be read via string name from Lua script
		frameGraph->ImportBufferResource("VertexBuffer", vertexBuffer[0], RHI::BufferUsage::Vertex);
		frameGraph->ImportBufferResource("IndexBuffer", indexBuffer[0], RHI::BufferUsage::Index);
		frameGraph->ImportBufferResource("UploadVertexBuffer", vertexUploadBuffer, RHI::BufferUsage::Copy);
		frameGraph->ImportBufferResource("UploadIndexBuffer", indexUploadBuffer, RHI::BufferUsage::Copy);

		rendererScript.LoadScriptFile("P:/Projects/VS/Wiley/Wiley/framegraph.lua");
		frameGraph->Compile();
	}

	Renderer::Renderer(Wiley::Window::Ref window, RHI::RenderContext::Ref rctx)
		:window(window), copyRingIndex(0), rctx(rctx)
	{
		ZoneScopedN("Renderer::Renderer");


		isVertexIndexDataDirty.fill(true);
		frameGraph = std::make_shared<FrameGraph>(rctx);

		InitializeScriptEngine();
		CreatePipelineState();
		
		RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
			L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/compute.hlsl", L"CSmain", nullptr);

		RHI::ComputePipelineSpecs cSpecs{};
		cSpecs.computeByteCode = computeByteCode;
		cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0, 3, 1 });
		cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 0, 1, 1 });
		cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 1, 1, 1 });
		cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 2, 1, 1 });
		cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 3, 1, 1 });
		cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 4, 1, 1 });
		computePso = RHI::ComputePipeline::CreateComputePipeline(rctx->GetDevice(), cSpecs,"ComputePipelineTest");

		sampler = rctx->CreateSampler(RHI::SamplerAddress::Clamp, RHI::SamplerFilter::Linear);
		postProcessSampler = rctx->CreateSampler(RHI::SamplerAddress::Clamp, RHI::SamplerFilter::Linear);
		gBufferReadSampler = rctx->CreateSampler(RHI::SamplerAddress::Clamp, RHI::SamplerFilter::Point);
		pbrSampler = rctx->CreateSampler(RHI::SamplerAddress::Wrap, RHI::SamplerFilter::Linear);

		const auto& heaps = rctx->GetDescriptorHeaps();

		uint32_t width = 0;
		uint32_t height = 0;
		window->GetClientDimensions(width, height);
		depthResource = std::make_shared<RHI::Texture>(rctx->GetDevice(), RHI::TextureFormat::D32, width, height, RHI::TextureUsage::DepthStencilTarget,rctx->GetDescriptorHeaps());

		vertexUploadBuffer = rctx->CreateUploadBuffer(FRAMES_IN_FLIGHT * maxVertexCount * sizeof(Wiley::Vertex), sizeof(Wiley::Vertex), "VertexUploadBuffer");
		indexUploadBuffer = rctx->CreateUploadBuffer(FRAMES_IN_FLIGHT * maxIndexCount * sizeof(uint32_t), sizeof(uint32_t), "IndexUploadBuffer");

		for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			std::string number = std::to_string(i);
			vertexBuffer[i] = rctx->CreateVertexBuffer(maxVertexCount * sizeof(Wiley::Vertex), sizeof(Wiley::Vertex), "VertexBuffer_" + number);
			indexBuffer[i] = rctx->CreateIndexBuffer(maxIndexCount * sizeof(uint32_t), sizeof(uint32_t), "IndexBuffer_" + number);
		}

		constantBuffer = rctx->CreateConstantBuffer(true, sizeof(Wiley::cBuffer));
		constantLightBuffer = rctx->CreateConstantBuffer(true, sizeof(Wiley::cBufferLight), "LightConstantBuffer");

		cBufferDesc = constantBuffer->GetCBV();
		cBufferLightDesc = constantLightBuffer->GetCBV();

		constantBuffer->Map(reinterpret_cast<void**>(&cPtr), 0, 0);
		constantLightBuffer->Map(reinterpret_cast<void**>(&cLightPtr), 0, 0);
		
		auto vg = rctx->CreateCubeMap(100, 100, RHI::TextureFormat::RGBA16);

		CompileFrameGraph();
		frameGraph->GetPassOrder();		
	}

	void Renderer::OnResize(uint32_t width, uint32_t height) {
		ZoneScopedN("Renderer::OnResize");

		UINT graphicsRingIndex = rctx->GetBackBufferIndex();

		rctx->GetGraphicsFence(graphicsRingIndex)->BlockGPU(rctx->GetCommandQueue().get());

		if (width == 0 || height == 0) {
			return;
		}

		{
			uint32_t _width, _height;
			window->GetClientDimensions(_width, _height);
			rctx->Resize(_width, _height);
		}

		viewportWidth = width;
		viewportHeight = height;

		frameGraph->OnResize(width, height);

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

	RHI::Texture::Ref Renderer::GetOutputTexture() const
	{
		return outputTexture;
	}

	FrameGraph::Ref Renderer::GetFrameGraph() const
	{
		return frameGraph;
	}

	void Renderer3D::Renderer::NewFrame(Wiley::Scene::Ref scene) {
		ZoneScopedN("Renderer::NewFrame");

		rctx->NewFrame();

		_scene = scene;

		if (_scene->IsVertexIndexDataDirty())
		{
			isVertexIndexDataDirty.fill(true);
			_scene->MakeVertexIndexDataClean();
		}

		camera = scene->GetCamera();

		viewMatrix = camera->GetView();
		projectionMatrix = camera->GetProjection();
		viewProjection = camera->GetViewProjection();

		//ImGui
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	void Renderer::EndFrame()
	{
		auto commandList = rctx->GetCurrentCommandList();

		rctx->TransitionSwapchain();
		auto backBuffer = rctx->GetBackBufferDescriptor();
		commandList->SetRenderTargets({ backBuffer }, RHI::DescriptorHeap::Descriptor::Invalid());
		commandList->ClearRenderTarget({ backBuffer }, { 1.0f,1.0f,1.0f,1.0f });

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList->GetCommandList());

		rctx->TransitionSwapchain();

		//Present + Render-to-final-texture pass close.
		{
			commandList->End();

			std::vector<RHI::CommandList::Ref> lists{ commandList };
			rctx->ExecuteGraphicsCommandList(lists);
		}

		rctx->GetCurrentGraphicsFence()->Signal(rctx->GetCommandQueue().get());
		rctx->GetCurrentGraphicsFence()->BlockCPU();
	}

	void Renderer::RenderToWindowDirect()
	{
		rctx->Present(false);
	}

	void Renderer::DrawCommands(RHI::CommandList::Ref commandList)
	{
		ZoneScopedN("Renderer::DrawCommands");

		for (int i = 0; i < drawCommandCache.size(); i++) {
			const DrawCommand& drawCmd = drawCommandCache[i];
			commandList->DrawInstancedIndexed(drawCmd.indexCount, drawCmd.instanceCount,
				drawCmd.indexStartLocation, drawCmd.vertexStartLocation, drawCmd.instanceStartIndex);
		}
	}

	void Renderer::DrawCommandsWithIndex(RHI::CommandList::Ref commandList)
	{
		ZoneScopedN("Renderer::DrawCommandsWithIndex");

		for (int i = 0; i < drawCommandCache.size(); i++) {
			const DrawCommand& drawCmd = drawCommandCache[i];

			commandList->PushConstant(&drawCmd.drawID, 4, 0);
			commandList->DrawInstancedIndexed(drawCmd.indexCount, drawCmd.instanceCount,
				drawCmd.indexStartLocation, drawCmd.vertexStartLocation, drawCmd.instanceStartIndex);
		}
	}


	void Renderer::RenderFrame()
	{
		ZoneScopedN("Renderer::RenderFrame");

		auto commandList = rctx->GetCurrentCommandList();

		frameGraph->Execute();
	
	}
}