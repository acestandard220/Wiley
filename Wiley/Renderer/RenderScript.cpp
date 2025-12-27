#include "Renderer.h"

namespace Renderer3D
{
	void Renderer::InitializeScriptEngine()
	{
		rendererScript.DefineEnum("render_pass_type",
			"graphics", RenderPassType::Graphics,
			"copy", RenderPassType::Copy,
			"compute", RenderPassType::Compute
		);

		rendererScript.DefineEnum("buffer_usage",
			"constant", RHI::BufferUsage::Constant,
			"vertex", RHI::BufferUsage::Vertex,
			"index", RHI::BufferUsage::Index,
			"compute_storage", RHI::BufferUsage::ComputeStorage,
			"copy", RHI::BufferUsage::Copy,
			"shader_resource", RHI::BufferUsage::ShaderResource,
			"non_pixel_shader_resource",RHI::BufferUsage::NonPixelShaderResource,
			"pixel_shader_resource",RHI::BufferUsage::PixelShaderResource,
			"read_back", RHI::BufferUsage::ReadBack
		);

		rendererScript.DefineEnum("texture_usage",
			"common", RHI::TextureUsage::Common,
			"copy_dest",RHI::TextureUsage::CopyDest,
			"depth_stencil_target",RHI::TextureUsage::DepthStencilTarget,
			"generic_read",RHI::TextureUsage::GenericRead,
			"present",RHI::TextureUsage::Present,
			"render_target",RHI::TextureUsage::RenderTarget,
			"shader_resource",RHI::TextureUsage::ShaderResource,
			"non_pixel_shader_resource",RHI::TextureUsage::NonPixelShader,
			"pixel_shader_resource",RHI::TextureUsage::PixelShaderResource,
			"depth_read",RHI::TextureUsage::DepthRead
		);

		rendererScript.DefineEnum("texture_format",
			"d24s8",RHI::TextureFormat::D24S8,
			"d32",RHI::TextureFormat::D32,
			"r16",RHI::TextureFormat::R16,
			"r16_unorm",RHI::TextureFormat::R16_UNORM,
			"r32",RHI::TextureFormat::R32,
			"r32t",RHI::TextureFormat::R32T,
			"rg16",RHI::TextureFormat::RG16,
			"rg16_unorm",RHI::TextureFormat::RG16_UNORM,
			"rg32",RHI::TextureFormat::RG32,
			"rg8_unorm",RHI::TextureFormat::RG8_UNORM,
			"rgb32",RHI::TextureFormat::RGB32,
			"rgba16",RHI::TextureFormat::RGBA16,
			"rgba16_unorm",RHI::TextureFormat::RGBA16_UNORM,
			"rgba32",RHI::TextureFormat::RGBA32,
			"rgba8_unorm",RHI::TextureFormat::RGBA8_UNORM,
			"unknown",RHI::TextureFormat::Unknown			
		);

		//rendererScript.SetFunction("geometry_pass_function", &Renderer::GeometryPass, this);
		auto& state = rendererScript.GetInternalState();

		state.set_function("geometry_pass_function", &Renderer::GeometryPass, this);
		state.set_function("wireframe_pass_function", &Renderer::WireframePass, this);
		state.set_function("shadow_pass_function", &Renderer::ShadowPass, this);
		state.set_function("present_pass_function", &Renderer::PresentPass, this);
		state.set_function("scene_copy_pass_function", &Renderer::SceneCopyPass, this);
		state.set_function("compute_scene_draw_pass_function", &Renderer::ComputeSceneDrawPass, this);

		state.set_function("cluster_generation_pass_function", &Renderer::ClusterGeneration, this);
		state.set_function("cluster_cull_pass_function", &Renderer::ClusterCullingPass, this);
		state.set_function("compact_cluster_pass_function", &Renderer::CompactClusterPass, this);
		state.set_function("cluster_assignment_pass_function", &Renderer::ClusterAssignmentPass, this);
		state.set_function("cluster_heatmap_pass_function", &Renderer::ClusterHeatMapPass, this);
		state.set_function("depth_prepass_function", &Renderer::DepthPrePass, this);
		state.set_function("lighting_pass_function", &Renderer::LightingPass, this);
		state.set_function("skybox_pass_function", &Renderer::SkyboxPass, this);


		rendererScript.DefineUserType<ResourceHandle>(
			"ResourceHandle",
			"id", &ResourceHandle::id,
			"set_texture_usage", [this](ResourceHandle& self, RHI::TextureUsage usage) {
				self.usage = usage;
			},
			"set_buffer_usage", [this](ResourceHandle& self, RHI::BufferUsage usage) {
				self.usage = usage;
			},
			"get_texture_usage", [this](ResourceHandle& self, RHI::TextureUsage usage) -> RHI::TextureUsage {
				try {
					return std::get<RHI::TextureUsage>(self.usage);
				}
				catch (std::bad_exception& e) {
					std::cout << "[Lua Script] :: Attempting to get non texture usage as texture usage." << std::endl;
				}
			},
			"get_buffer_usage", [this](ResourceHandle& self, RHI::BufferUsage usage) -> RHI::BufferUsage {
				try {
					return std::get<RHI::BufferUsage>(self.usage);
				}
				catch (std::bad_exception& e) {
					std::cout << "[Lua Script] :: Attempting to get non buffer usage as buffer usage." << std::endl;
				}
			}
		);

		rendererScript.DefineUserType<RenderPass>(
			"RenderPass",
			sol::constructors<RenderPass()>(),
			"set_name", [this](RenderPass& pass, const std::string& name) {
				pass.name = name;
			},
			"set_type", [this](RenderPass& pass, RenderPassType type) {
				pass.passType = type;
			},
			"set_viewport", [this](RenderPass& pass, UINT width, UINT height)
			{
				pass.passDimensions = { static_cast<float>(width), static_cast<float>(height) ,0 };
			},
			"create_texture", [this](RenderPass& pass, const std::string name, RHI::TextureFormat format, uint32_t width, uint32_t height, RHI::TextureUsage usage, RHI::TextureUsage transition, bool isScreenSizeDependent = true) {
				auto handle = frameGraph->CreateTextureResource(name, {
					.format = format,.width = width,.height = height,.usage = usage
					}, transition, isScreenSizeDependent);
				pass.outputs.push_back(handle);
			},
			"create_buffer", [this](RenderPass& pass, const std::string& name, UINT64 size, UINT64 stride, RHI::BufferUsage usage, bool persistent, RHI::BufferUsage transition) {
				auto handle = frameGraph->CreateBufferResource(name, {
					.size = size,.stride = stride,.usage = usage,.persistent = persistent
					}, transition);
				pass.outputs.push_back(handle);
			},
			"create_input_buffer", [this](RenderPass& pass, const std::string& name, UINT64 size, UINT64 stride, RHI::BufferUsage usage, bool persistent, RHI::BufferUsage transition){
				auto handle = frameGraph->CreateBufferResource(name, {
					.size = size,.stride = stride,.usage = usage,.persistent = persistent
					}, transition);
				pass.inputs.push_back(handle);
			},
			"import_texture", [this](RenderPass& pass, const std::string& name, RHI::Texture::Ref texture, RHI::TextureUsage usage) {
				auto handle = frameGraph->ImportTextureResource(name, texture, usage);
				pass.outputs.push_back(handle);
			},
			"read_texture", [this](RenderPass& pass, const std::string& name, RHI::TextureUsage transition) {
				auto handle = frameGraph->ReadTextureResource(name, transition);
				pass.inputs.push_back(handle);
			},
			"read_buffer", [this](RenderPass& pass, const std::string& name, RHI::BufferUsage transition) {
				auto handle = frameGraph->ReadBufferResource(name, transition);
				pass.inputs.push_back(handle);
			},
			"write_texture", [this](RenderPass& pass, const std::string& name, RHI::TextureUsage transition)
			{
				auto handle = frameGraph->WriteTextureResource(name, transition);
				pass.outputs.push_back(handle);
			},
			"write_buffer", [this](RenderPass& pass, const std::string& name, RHI::BufferUsage transition)
			{
				auto handle = frameGraph->WriteBufferResource(name, transition);
				pass.outputs.push_back(handle);
			}, 
			"execute", [this](RenderPass& pass,std::function<void(RenderPass&)> passExecution) {
				pass.execute = passExecution;
			}
		);

		rendererScript.SetFunction("add_pass", [this](RenderPass pass) {
			frameGraph->AddPass(pass);
		});

		rendererScript.SetFunction("frame_width", [this]()->uint32_t{
			uint32_t width, height;
			window->GetDimensions(width, height);
			return width;
		});

		rendererScript.SetFunction("frame_height", [this]()->uint32_t {
			uint32_t width, height;
			window->GetDimensions(width, height);
			return height;
		});

		rendererScript.SetConstant("max_index", MAX_INDEX_COUNT);
		rendererScript.SetConstant("max_light_count", MAX_LIGHTS);
		rendererScript.SetConstant("max_vertex", MAX_VERTEX_COUNT);
		rendererScript.SetConstant("max_mesh_count", MAX_MESH_COUNT);
		rendererScript.SetConstant("max_submesh_count", MAX_SUBMESH_COUNT);
		rendererScript.SetConstant("max_material_count", MAX_MATERIAL_COUNT);

		rendererScript.SetConstant("int_size", WILEY_SIZEOF(int));
		rendererScript.SetConstant("uint_size", WILEY_SIZEOF(UINT));

		rendererScript.SetConstant("vertex_size", WILEY_SIZEOF(Wiley::Vertex));
		rendererScript.SetConstant("submesh_data_size", WILEY_SIZEOF(Wiley::SubMeshData));
		rendererScript.SetConstant("material_data_size", WILEY_SIZEOF(Wiley::MaterialData));
		rendererScript.SetConstant("mesh_instance_base_size", WILEY_SIZEOF(Wiley::MeshInstanceBase));

		rendererScript.SetConstant("mesh_filter_size", WILEY_SIZEOF(Wiley::MeshFilterComponent));
		rendererScript.SetConstant("light_component_size", WILEY_SIZEOF(Wiley::LightComponent));
		rendererScript.SetConstant("sorted_light_size", WILEY_SIZEOF(Wiley::SortedLight));//

		struct Cluster
		{
			DirectX::XMFLOAT3 _min;
			DirectX::XMFLOAT3 _max;
		};
		struct ClusterData {
			uint32_t size;
			uint32_t offset;
		};
		rendererScript.SetConstant("cluster_size", WILEY_SIZEOF(Cluster));
		rendererScript.SetConstant("tile_size", TILE_GRID_SIZE);
		rendererScript.SetConstant("cluster_depth", CLUSTER_DEPTH);
		rendererScript.SetConstant("cluster_data_size", WILEY_SIZEOF(ClusterData));
		rendererScript.SetConstant("max_light_per_cluster", MAX_LIGHT_PER_CLUSTER);
	}

}