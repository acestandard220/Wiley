#include "../Renderer.h"

namespace Renderer3D {

	/// <summary>
	///		Shader Entry Points are VSmain PSmain & CSmain in shader files if they only contain one shader entry point.
	/// </summary>
	void Renderer::CreatePipelineState()
	{
		ZoneScopedN("Renderer::CreatePipelines");

		//Cluster Generaion
		{

			RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/cluster_gen.hlsl", L"GenerateClusters", nullptr);

			RHI::ComputePipelineSpecs cSpecs{};
			cSpecs.computeByteCode = computeByteCode;

			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange,0,1,0,RHI::ShaderVisibility::Compute});
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange,0,1,1,RHI::ShaderVisibility::Compute});

			auto pso = RHI::ComputePipeline::CreateComputePipeline(rctx->GetDevice(), cSpecs, "GenClusterPipeline");
			computePsoCache[RenderPassSemantic::GenClusterPass] = pso;
		}

		//Cluster Cull
		{

			RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/cluster_cull.hlsl", L"ClusterCulling", nullptr);

			RHI::ComputePipelineSpecs cSpecs{};
			cSpecs.computeByteCode = computeByteCode;

			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange,0,1,0,RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange,0,1,1,RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange,1,1,1,RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange,2,1,1,RHI::ShaderVisibility::Compute });

			auto pso = RHI::ComputePipeline::CreateComputePipeline(rctx->GetDevice(), cSpecs, "ClusterCullPipeline");
			computePsoCache[RenderPassSemantic::ClusterCullPass] = pso;
		}

		//Cluster Compact
		{

			RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/compact_cluster.hlsl", L"CompactClusters", nullptr);

			RHI::ComputePipelineSpecs cSpecs{};
			cSpecs.computeByteCode = computeByteCode;

			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 0, 1, 0, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 0, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 1, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 2, 1, 1, RHI::ShaderVisibility::Compute });

			auto pso = RHI::ComputePipeline::CreateComputePipeline(rctx->GetDevice(), cSpecs, "CompactClustersPipeline");
			computePsoCache[RenderPassSemantic::CompactClusterPass] = pso;
		}

		//Cluster Assigment
		{

			RHI::ShaderByteCode computeByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Compute,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/cluster_assignment.hlsl", L"ClusterAssignment", nullptr);

			RHI::ComputePipelineSpecs cSpecs{};
			cSpecs.computeByteCode = computeByteCode;

			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 0, 1, 0, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 0, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 1, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 2, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 3, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 4, 1, 1, RHI::ShaderVisibility::Compute });
			cSpecs.rootSpecs.entries.push_back({ RHI::RootSignatureEntryType::UAVRange, 5, 1, 1, RHI::ShaderVisibility::Compute });

			auto pso = RHI::ComputePipeline::CreateComputePipeline(rctx->GetDevice(), cSpecs, "ClusterAssignmentPipeline");
			computePsoCache[RenderPassSemantic::ClusterAssignment] = pso;
		}

		//Cluster HeatMap
		{
			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/cluster_heatmap.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/cluster_heatmap.hlsl", L"ClusterCullHeatMapmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = false;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::Back;

			specs.nRenderTarget = 1;
			specs.textureFormats[0] = RHI::TextureFormat::RGBA16;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 0 ,1,0});
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0 ,1,1});//b1
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1 ,1,1});//t1


			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::ClusterHeapMapPass] = rctx->CreateGraphicsPipeline(specs);
		}


		//Geometry Pass PSO
		{
			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/shader.hlsl",L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/shader.hlsl", L"PSmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = true;
			specs.depthFunc = RHI::DepthFunc::LessEqual;
			specs.depthWrite = true;
			specs.depthStencilFormat = RHI::TextureFormat::D32;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::Back;

			specs.nRenderTarget = 4;
			specs.textureFormats[0] = RHI::TextureFormat::RGBA16;
			specs.textureFormats[1] = RHI::TextureFormat::RGBA16;
			specs.textureFormats[2] = RHI::TextureFormat::RGBA16;
			specs.textureFormats[3] = RHI::TextureFormat::RGBA16;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 1, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 2, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 3, 1, 0, RHI::ShaderVisibility::Vertex });

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 4, 1, 0, RHI::ShaderVisibility::Pixel });//Mtl_Data
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 5,1,0,RHI::ShaderVisibility::Pixel });//sampler

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0, MAX_IMAGETEXTURE_COUNT , 1, RHI::ShaderVisibility::Pixel });//t5 .albedo
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0, MAX_IMAGETEXTURE_COUNT , 2, RHI::ShaderVisibility::Pixel });//t5 .normal
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0, MAX_IMAGETEXTURE_COUNT , 3, RHI::ShaderVisibility::Pixel });//t5 .arm

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0, 1, 4 ,RHI::ShaderVisibility::Vertex});
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1, 1, 4 ,RHI::ShaderVisibility::Vertex});


			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::Geometry] = rctx->CreateGraphicsPipeline(specs);
		}

		//Depth Pre-Pass
		{
			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/depth_prepass.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/depth_prepass.hlsl", L"EmptyPixelShader", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = true;
			specs.depthWrite = true;
			specs.depthFunc = RHI::DepthFunc::Less;
			specs.depthStencilFormat = RHI::TextureFormat::D32;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::None;

			specs.nRenderTarget = 0;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 1, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 2, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 3, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 4, 1, 0, RHI::ShaderVisibility::Vertex });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 5, 1, 0, RHI::ShaderVisibility::Vertex });

			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::DepthPrepass] = rctx->CreateGraphicsPipeline(specs);
		}

		//Wireframe Pass
		{
			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/debug.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/debug.hlsl", L"PSmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = true;
			specs.depthStencilFormat = RHI::TextureFormat::D32;

			specs.line = true;
			specs.fillMode = RHI::FillMode::WireFrame;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::None;

			specs.nRenderTarget = 1;
			specs.textureFormats[0] = RHI::TextureFormat::RGBA16;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0 });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 1 });

			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::Wireframe] = rctx->CreateGraphicsPipeline(specs);
		}

		//Point Shadow Pass PSO
		{
			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/shadow_map.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/shadow_map.hlsl", L"PSmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = true;
			specs.depthStencilFormat = RHI::TextureFormat::D32;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::None;

			specs.nRenderTarget = 1;
			specs.textureFormats[0] = RHI::TextureFormat::R32;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0, 8,0, RHI::ShaderVisibility::Vertex });

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0, 1, 1, RHI::ShaderVisibility::Vertex});
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1, 1, 1, RHI::ShaderVisibility::Vertex});
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 2, 1, 1, RHI::ShaderVisibility::Vertex});
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 3, 1, 1, RHI::ShaderVisibility::Vertex});

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0, 1, 2, RHI::ShaderVisibility::Vertex });

			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::PointShadowMapPass] = rctx->CreateGraphicsPipeline(specs);
		}

		//Skybox Pass
		{

			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/Lighting/skybox.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/Lighting/skybox.hlsl", L"PSmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = true;
			specs.depthFunc = RHI::DepthFunc::LessEqual;
			specs.depthStencilFormat = RHI::TextureFormat::D32;
			specs.depthWrite = false;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::None;

			specs.nRenderTarget = 1;
			specs.textureFormats[0] = RHI::TextureFormat::RGBA16;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0,36,0,RHI::ShaderVisibility::All });

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0,1,1,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 1,1,1,RHI::ShaderVisibility::Pixel });

			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::SkyboxPass] = rctx->CreateGraphicsPipeline(specs);
		}

		//Lighting Pass
		{

			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/Lighting/lighting.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/Lighting/lighting.hlsl", L"PSmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = false;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::None;

			specs.nRenderTarget = 1;
			specs.textureFormats[0] = RHI::TextureFormat::RGBA16;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 0,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 2,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 3,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 4,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 5,1,0,RHI::ShaderVisibility::Pixel });

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::Constant, 0,6,1,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1,1,1,RHI::ShaderVisibility::Pixel });

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 2, MAX_LIGHTS,2,RHI::ShaderVisibility::Pixel }); //CubeDepths
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 3, MAX_LIGHTS,3,RHI::ShaderVisibility::Pixel }); //ArrayDepths
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 4, 1,4,RHI::ShaderVisibility::Pixel }); //LightVPs
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 5,1,5,RHI::ShaderVisibility::Pixel }); //DepthSamplerNoComp

			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::LightingPass] = rctx->CreateGraphicsPipeline(specs);
		}

		//Present Pass
		{

			RHI::ShaderByteCode vertexByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Vertex,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/present.hlsl", L"VSmain", nullptr);
			RHI::ShaderByteCode pixelByteCode = RHI::ShaderCompiler::CompileShader(RHI::ShaderType::Pixel,
				L"P:/Projects/VS/Wiley/Wiley/Assets/Shaders/present.hlsl", L"PSmain", nullptr);

			std::unordered_map<RHI::ShaderType, RHI::ShaderByteCode> shaders{
				{ RHI::ShaderType::Vertex, vertexByteCode},
				{ RHI::ShaderType::Pixel, pixelByteCode}
			};

			RHI::GraphicsPipelineSpecs specs{};
			specs.depth = false;
			specs.depthStencilFormat = RHI::TextureFormat::D32;

			specs.line = false;
			specs.fillMode = RHI::FillMode::Solid;
			specs.frontFace = RHI::FrontFace::CounterClockWise;
			specs.cullMode = RHI::CullMode::None;

			specs.nRenderTarget = 1;
			specs.textureFormats[0] = RHI::TextureFormat::RGBA8_UNORM;

			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::CBVRange, 0,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SRVRange, 1,1,0,RHI::ShaderVisibility::Pixel });
			specs.rootSignatureSpecs.entries.push_back({ RHI::RootSignatureEntryType::SamplerRange, 2,1,0,RHI::ShaderVisibility::Pixel });

			specs.byteCodes = shaders;
			gfxPsoCache[RenderPassSemantic::PresentPass] = rctx->CreateGraphicsPipeline(specs);
		}
	}


}