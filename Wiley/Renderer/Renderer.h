#pragma once
//Created on 11/9/2025

#include "../RHI/RenderContext.h"

#include "../Core/ScriptEngine.h"

#include "FrameGraph.h"
#include "../Scene/Scene.h"



#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"

#include "Tracy/tracy/Tracy.hpp"
#include "../Core/TracyWrapper.h"

#include <cstdint>

#define TILE_GRID_SIZE 32
#define CLUSTER_DEPTH 32
#define MAX_LIGHT_PER_CLUSTER 64


namespace Renderer3D
{
	struct FrameStatistics {
		UINT meshCount = 0;
		UINT subMeshCount = 0;

		UINT64 vertexCount = 0;
		UINT64 indexCount = 0;

		UINT activeClusterCount = 0;
	};

	struct DrawCommand {
		std::uint32_t drawID = 0;

		std::uint32_t indexCount = 0;
		std::uint32_t instanceCount = 0;
		std::uint32_t indexStartLocation = 0;
		std::uint32_t vertexStartLocation = 0;
		std::uint32_t instanceStartIndex = 0;
	};

	class Renderer
	{
	public:
		using Ref = std::shared_ptr<Renderer>;
		using Ptr = std::unique_ptr<Renderer>;

		Renderer(Wiley::Window::Ref window, RHI::RenderContext::Ref rctx);
		~Renderer() {

		}

		void InitializeScriptEngine();
		void CompileFrameGraph();

		void CreatePipelineState();

		void NewFrame(Wiley::Scene::Ref scene);
		void EndFrame();
		void RenderToWindowDirect();
		void DrawCommands(RHI::CommandList::Ref);

		//Render Pass Execution Functions
		void WireframePass(RenderPass& pass);
		void DepthPrePass(RenderPass& pass);
		void GeometryPass(RenderPass& pass);
		void ShadowPass(RenderPass& pass);
		void PresentPass(RenderPass& pass);
		void SceneCopyPass(RenderPass& pass);
		void LightingPass(RenderPass& pass);
		void SkyboxPass(RenderPass& pass);
		void ShadowMapPass(RenderPass& pass);
		//Compute Executions
		void ComputeSceneDrawPass(RenderPass& pass);

		void ClusterGeneration(RenderPass& pass);
		void ClusterCullingPass(RenderPass& pas);
		void CompactClusterPass(RenderPass& pass);
		void ClusterAssignmentPass(RenderPass& pass);
		void ClusterHeatMapPass(RenderPass& pass);


		void RenderFrame();
		void OnResize(uint32_t width, uint32_t height);

		RHI::Texture::Ref GetOutputTexture()const;
		FrameGraph::Ref GetFrameGraph()const;

	private:
		std::vector<DrawCommand> drawCommandCache;
		RHI::ComputePipeline::Ref computePso;

		RHI::DescriptorHeap::Descriptor cBufferDesc;
		RHI::DescriptorHeap::Descriptor cBufferLightDesc;

		RHI::Sampler::Ref sampler;
		RHI::Sampler::Ref postProcessSampler;
		RHI::Sampler::Ref pbrSampler;
		RHI::Sampler::Ref gBufferReadSampler;


		RHI::Texture::Ref  shaderTexture;

		//UINT graphicsRingIndex;
		UINT copyRingIndex;
		std::array<bool, FRAMES_IN_FLIGHT> isVertexIndexDataDirty;

		//UINT nIndices;
		static const size_t maxVertexCount = MAX_VERTEX_COUNT;
		static const size_t maxIndexCount = MAX_INDEX_COUNT;

		RHI::Buffer::Ref vertexBuffer[FRAMES_IN_FLIGHT];
		RHI::Buffer::Ref indexBuffer[FRAMES_IN_FLIGHT];

		RHI::Buffer::Ref constantBuffer;
		RHI::Buffer::Ref constantLightBuffer;

		RHI::Buffer::Ref vertexUploadBuffer;
		RHI::Buffer::Ref indexUploadBuffer;
		
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMMATRIX viewProjection;
		Wiley::Camera::Ref camera;

		UINT8* cPtr;
		UINT8* cLightPtr;

		uint32_t viewportWidth, viewportHeight;
		RHI::Texture::Ref depthResource;
		RHI::Texture::Ref outputTexture;

		std::unordered_map<RenderPassSemantic, RHI::GraphicsPipeline::Ref> gfxPsoCache;
		std::unordered_map<RenderPassSemantic, RHI::ComputePipeline::Ref> computePsoCache;

		ScriptState rendererScript;

		Wiley::Window::Ref window;
		RHI::RenderContext::Ref rctx;
		FrameGraph::Ref frameGraph;


		Wiley::Scene::Ref _scene;

		FrameStatistics statistics;
	};
}

