#pragma once
#include "FrameGraph.h"

namespace Renderer3D {

	class DebugRenderer
	{
		using Vector3 = DirectX::XMFLOAT3;
		using Vector2 = DirectX::XMFLOAT2;
		using Bool = bool;
		
		/// <summary>
		/// Raw Vertex Data sent to the GPU.
		/// </summary>
		struct DebugVertex {
			Vector3 position;
			Vector3 color;
		};

		struct DebugLine {
			DebugVertex start;
			DebugVertex end;
		};

		struct DebugTriangle {
			DebugVertex a;
			DebugVertex b;
			DebugVertex c;
		};

		public:
			DebugRenderer(RHI::RenderContext::Ref rctx, FrameGraph* frameGraph)
				:rctx(rctx)
			{
				
				RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
					L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/.hlsl", L"VSmain", nullptr);
				RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
					L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/.hlsl", L"PSmain", nullptr);

				std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
					{ RHI::ShaderType::Vertex, vertexByteCode},
					{ RHI::ShaderType::Pixel, pixelByteCode}
				};

				RHI::GraphicsPipelineSpecs specs{};
				specs.depth = true;
				specs.depthStencilFormat = RHI::TextureFormat::D32;

				specs.line = false;
				specs.fillMode = RHI::FillMode::WireFrame;
				specs.frontFace = RHI::FrontFace::CounterClockWise;
				specs.cullMode = RHI::CullMode::None;

				specs.nRenderTarget = 1;
				specs.textureFormats[0] = RHI::TextureFormat::RGBA8_UNORM;

				specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0 });
				specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 1 });

				specs.byteCodes = shaders;

				gfxPipelineState = rctx->CreateGraphicsPipeline(specs);
				
				uploadDebugVertexBuffer = rctx->CreateUploadBuffer(WILEY_SIZEOF(DebugVertex) * maxVertexCount, WILEY_SIZEOF(DebugVertex), "UploadDebugVertexBuffer");
				debugVertexBuffer = rctx->CreateVertexBuffer(WILEY_SIZEOF(DebugVertex) * maxVertexCount, WILEY_SIZEOF(DebugVertex), "DebugVertexBuffer");

			}

			void AddLine(Vector3 const& start, Vector3 const& end, Vector3 const& color = { 1.0f,0.0f,0.0f });
			void AddTriangle(Vector3 const& a, Vector3 const& b, Vector3 const& c);

			void AddAABB(Vector3 min, Vector3 max, Vector3 color = { 1.0,0.0f,0.0f }, Bool wireFrame = true);

			void DebugRender(RenderPass& pass);;

		private:
			const int maxVertexCount = 100'000;

			std::vector<DebugLine> debugLines;
			std::vector<DebugTriangle> debugTriangle;

			RHI::GraphicsPipeline::Ref gfxPipelineState;
			RHI::Buffer::Ref uploadDebugVertexBuffer;
			RHI::Buffer::Ref debugVertexBuffer;

			RHI::RenderContext::Ref rctx;
	};

}