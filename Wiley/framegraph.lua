local width = frame_width()
local height = frame_height()

local clusterCountX = (width + tile_size - 1) // tile_size
local clusterCountY = (height + tile_size - 1) // tile_size
local clusterCountZ = cluster_depth
local clusterCount = clusterCountX * clusterCountY * clusterCountZ

--xxxx_pass:create_texture Automatically records it as an output. No need to specify it as a write.

--setting the vertex and index buffer as inputs for every other pass guarantees they happen after the copy.
	
local scene_copy_pass = RenderPass.new()
	scene_copy_pass:set_name("SceneCopyPass")
	scene_copy_pass:set_type(render_pass_type.copy)
	
	--Input Resources
	scene_copy_pass:read_buffer("UploadVertexBuffer",buffer_usage.copy)
	scene_copy_pass:read_buffer("UploadIndexBuffer",buffer_usage.copy)

	--Output Resources
	scene_copy_pass:write_buffer("VertexBuffer",buffer_usage.vertex)
	scene_copy_pass:write_buffer("IndexBuffer",buffer_usage.vertex)

	scene_copy_pass:create_buffer("UploadMaterialDataBuffer", material_data_size * max_material_count, material_data_size, buffer_usage.copy, true, buffer_usage.copy)
	scene_copy_pass:create_buffer("MaterialDataBuffer", material_data_size * max_material_count, material_data_size, buffer_usage.pixel_shader_resource, true, buffer_usage.pixel_shader_resource)

	--This is a special buffer that will used zero out counter buffers.
	scene_copy_pass:execute(scene_copy_pass_function)
add_pass(scene_copy_pass)

local compute_scene_draw_pass = RenderPass.new()
	compute_scene_draw_pass:set_name("ComputeSceneDrawPass")
	compute_scene_draw_pass:set_type(render_pass_type.compute)
	
	--Input Resources

	--Output Resources
	compute_scene_draw_pass:create_buffer("UploadMeshFilterBuffer", mesh_filter_size * max_mesh_count, mesh_filter_size, buffer_usage.copy, true, buffer_usage.copy)
	compute_scene_draw_pass:create_buffer("MeshFilterBuffer", mesh_filter_size * max_mesh_count, mesh_filter_size, buffer_usage.compute_storage, false, buffer_usage.compute_storage)

	compute_scene_draw_pass:create_buffer("UploadSubMeshDataBuffer", submesh_data_size * max_submesh_count, submesh_data_size,buffer_usage.copy, true, buffer_usage.copy)
	compute_scene_draw_pass:create_buffer("SubMeshDataBuffer", submesh_data_size * max_submesh_count, submesh_data_size,buffer_usage.shader_resource, false, buffer_usage.shader_resource)

	compute_scene_draw_pass:create_buffer("MeshCountBuffer", uint_size, uint_size, buffer_usage.compute_storage, true, buffer_usage.compute_storage)
	compute_scene_draw_pass:create_buffer("ReadBackMeshCountBuffer", uint_size, uint_size, buffer_usage.read_back, true, buffer_usage.read_back)

	compute_scene_draw_pass:create_buffer("UploadMeshInstanceBaseBuffer", mesh_instance_base_size * max_mesh_count, mesh_instance_base_size, buffer_usage.copy, true, buffer_usage.copy)
	compute_scene_draw_pass:create_buffer("MeshInstanceBaseBuffer", mesh_instance_base_size * max_mesh_count, mesh_instance_base_size, buffer_usage.compute_storage, false, buffer_usage.compute_storage)

	compute_scene_draw_pass:create_buffer("UploadMeshInstanceIndexBuffer_PreOcclusion",uint_size * max_mesh_count, uint_size, buffer_usage.copy, true, buffer_usage.copy)
	compute_scene_draw_pass:create_buffer("MeshInstanceIndexBuffer_PreOcclusion",uint_size * max_mesh_count,uint_size,buffer_usage.compute_storage,false,buffer_usage.compute_storage)

	compute_scene_draw_pass:create_buffer("MeshInstanceIndexBuffer_PostOcclusion",uint_size * max_mesh_count,uint_size,buffer_usage.compute_storage,false,buffer_usage.compute_storage)
	compute_scene_draw_pass:create_buffer("ReadBackMeshInstanceIndexBuffer_PostOcclusion",uint_size * max_mesh_count,uint_size,buffer_usage.read_back,false,buffer_usage.read_back)
	compute_scene_draw_pass:create_buffer("ReadBackMeshInstanceBaseBuffer", mesh_instance_base_size * max_mesh_count, mesh_instance_base_size, buffer_usage.read_back, true, buffer_usage.read_back)


	compute_scene_draw_pass:execute(compute_scene_draw_pass_function)
add_pass(compute_scene_draw_pass)

local depth_prepass = RenderPass.new()
	depth_prepass:set_name("DepthPrepass")
	depth_prepass:set_type(render_pass_type.graphics)

	--Input Resources
	depth_prepass:read_buffer("MeshFilterBuffer",buffer_usage.non_pixel_shader_resource)
	depth_prepass:read_buffer("SubMeshDataBuffer",buffer_usage.non_pixel_shader_resource)
	depth_prepass:create_input_buffer("DepthPrepassCBuffer", 256, 256, buffer_usage.constant, true, buffer_usage.constant)
	depth_prepass:read_buffer("MeshInstanceIndexBuffer_PostOcclusion", buffer_usage.read_back)
	depth_prepass:read_buffer("MeshInstanceBaseBuffer", buffer_usage.read_back)

	--Out Resources
	depth_prepass:create_texture("DepthPrepassBuffer",texture_format.d32,width,height,texture_usage.depth_stencil_target,texture_usage.depth_stencil_target,true)
	
	depth_prepass:execute(depth_prepass_function)
add_pass(depth_prepass)

local cluster_generation_pass = RenderPass.new()
	cluster_generation_pass:set_name("ClusterGenerationPass")
	cluster_generation_pass:set_type(render_pass_type.compute)

	--Input Resources

	--Output Resources
	cluster_generation_pass:create_buffer("ClusterGenerationConstants", 256, 256, buffer_usage.constant, true, buffer_usage.constant)
	cluster_generation_pass:create_buffer("ClusterBuffer", cluster_size * clusterCount, cluster_size, buffer_usage.compute_storage, true, buffer_usage.compute_storage)

	cluster_generation_pass:execute(cluster_generation_pass_function)
add_pass(cluster_generation_pass)

local cluster_cull_pass = RenderPass.new()
	cluster_cull_pass:set_name("ClusterCullPass")
	cluster_cull_pass:set_type(render_pass_type.compute)

	--Input Resources
	cluster_cull_pass:read_texture("DepthPrepassBuffer", texture_usage.shader_resource)
	cluster_cull_pass:create_input_buffer("ResetActiveClusters",uint_size * clusterCount,uint_size,buffer_usage.copy,true,buffer_usage.copy)

	--Output Resources
	cluster_cull_pass:create_buffer("ClusterCullConstants", 256, 256, buffer_usage.constant, true, buffer_usage.constant)
	cluster_cull_pass:create_buffer("ActiveClusters", uint_size * clusterCount, uint_size, buffer_usage.compute_storage, true, buffer_usage.compute_storage)

	cluster_cull_pass:execute(cluster_cull_pass_function)
add_pass(cluster_cull_pass)

local compact_cluster_cull_pass = RenderPass.new()
	compact_cluster_cull_pass:set_name("CompactClusterCullPass")
	compact_cluster_cull_pass:set_type(render_pass_type.compute)

	--Input Resources
	compact_cluster_cull_pass:read_buffer("ActiveClusters",buffer_usage.compute_storage)
	compact_cluster_cull_pass:read_buffer("ClusterCullConstants",buffer_usage.constant) --using same buffer for simplicity

	--Output Resources
	compact_cluster_cull_pass:create_buffer("ActiveClusterCount", uint_size, uint_size, buffer_usage.compute_storage, true, buffer_usage.compute_storage)
	compact_cluster_cull_pass:create_buffer("ActiveClusterIndex", uint_size * clusterCount, uint_size, buffer_usage.compute_storage, true, buffer_usage.compute_storage)
	compact_cluster_cull_pass:create_buffer("ReadBackActiveClusterCount", uint_size, uint_size, buffer_usage.read_back, true, buffer_usage.read_back)

	compact_cluster_cull_pass:execute(compact_cluster_pass_function)
add_pass(compact_cluster_cull_pass)

local cluster_assignment_pass = RenderPass.new()
	cluster_assignment_pass:set_name("ClusterAssignmentPass")
	cluster_assignment_pass:set_type(render_pass_type.compute)

	--Input Resources
	cluster_assignment_pass:read_buffer("ActiveClusterIndex",buffer_usage.compute_storage)
	cluster_assignment_pass:read_buffer("ClusterBuffer",buffer_usage.compute_storage)
	cluster_assignment_pass:create_input_buffer("UploadLightCullDataBuffer", (uint_size * 4) * max_light_count,(uint_size * 4),buffer_usage.copy,false,buffer_usage.copy)
	cluster_assignment_pass:create_input_buffer("ClusterAssignCBuffer", 256,256,buffer_usage.constant, true, buffer_usage.constant)
	cluster_assignment_pass:read_buffer("ReadBackActiveClusterCount",buffer_usage.copy)
	cluster_assignment_pass:create_input_buffer("UploadLightCompBuffer", light_component_size * max_light_count,light_component_size,buffer_usage.copy, false, buffer_usage.copy)

	--Output Resources
	cluster_assignment_pass:create_buffer("LightCullDataBuffer", (uint_size * 4) * max_light_count,(uint_size * 4),buffer_usage.compute_storage,false,buffer_usage.compute_storage)
	cluster_assignment_pass:create_buffer("LightGridPtr", uint_size,uint_size,buffer_usage.compute_storage,false,buffer_usage.compute_storage)

	cluster_assignment_pass:create_buffer("ClusterDataBuffer", cluster_data_size * clusterCount, cluster_data_size,buffer_usage.compute_storage,false,buffer_usage.compute_storage)
	cluster_assignment_pass:create_buffer("LightGridBuffer", uint_size * max_light_per_cluster * max_light_count, uint_size, buffer_usage.compute_storage,false,buffer_usage.compute_storage)

	cluster_assignment_pass:create_buffer("LightCompBuffer", light_component_size * max_light_count, light_component_size, buffer_usage.compute_storage, false, buffer_usage.compute_storage)

	cluster_assignment_pass:execute(cluster_assignment_pass_function)
add_pass(cluster_assignment_pass)

local cluster_heatmap_pass = RenderPass.new()
	cluster_heatmap_pass:set_name("ClusterHeatMapPass")
	cluster_heatmap_pass:set_type(render_pass_type.graphics)

	--Input Resources
	cluster_heatmap_pass:read_buffer("ClusterDataBuffer", buffer_usage.shader_resource)
	cluster_heatmap_pass:read_texture("DepthPrepassBuffer", buffer_usage.shader_resource)
	cluster_heatmap_pass:create_input_buffer("ClusterHeatMapCBuffer", 256, 256, buffer_usage.constant, true, buffer_usage.constant)

	--OuputResources
	cluster_heatmap_pass:create_texture("ClusterHeatMap", texture_format.rgba16, width, height, texture_usage.present, texture_usage.render_target, true)

	cluster_heatmap_pass:execute(cluster_heatmap_pass_function)
add_pass(cluster_heatmap_pass)

local geometry_pass = RenderPass.new()
	geometry_pass:set_name("GeometryPass")
	geometry_pass:set_type(render_pass_type.graphics)
	geometry_pass:set_viewport(width,height)

	--Input Resources
	geometry_pass:read_buffer("MeshFilterBuffer", buffer_usage.non_pixel_shader_resource)
	geometry_pass:read_buffer("SubMeshDataBuffer",buffer_usage.non_pixel_shader_resource)
	geometry_pass:read_buffer("MaterialDataBuffer",buffer_usage.pixel_shader_resource)
	geometry_pass:read_buffer("MeshInstanceIndexBuffer_PostOcclusion", buffer_usage.read_back)
	geometry_pass:read_buffer("MeshInstanceBaseBuffer", buffer_usage.read_back)

	--Output Resources
	geometry_pass:create_texture("PositionalData",texture_format.rgba16,width,height,texture_usage.present,texture_usage.render_target,true)
	geometry_pass:create_texture("NormalData",texture_format.rgba16,width,height,texture_usage.present,texture_usage.render_target,true)
	geometry_pass:create_texture("ColorData",texture_format.rgba16,width,height,texture_usage.present,texture_usage.render_target,true)
	geometry_pass:create_texture("ArmData",texture_format.rgba16,width,height,texture_usage.present,texture_usage.render_target,true)
	geometry_pass:create_texture("DepthData",texture_format.d32,width,height,texture_usage.depth_stencil_target,texture_usage.depth_stencil_target,true)

	geometry_pass:execute(geometry_pass_function)
add_pass(geometry_pass)

local wire_frame_pass = RenderPass.new()
	wire_frame_pass:set_name("WireframePass")
	wire_frame_pass:set_type(render_pass_type.graphics)
	wire_frame_pass:set_viewport(width,height)

	--Output Resources
	wire_frame_pass:create_texture("WireframeData",texture_format.rgba16,width,height,texture_usage.present,texture_usage.render_target,true)

	wire_frame_pass:execute(wireframe_pass_function)
--add_pass(wire_frame_pass)

local shadow_pass = RenderPass.new()
	shadow_pass:set_name("ShadowPass")
	shadow_pass:set_type(render_pass_type.graphics)
	shadow_pass:set_viewport(width,height)

	--Output Resources
	shadow_pass:create_texture("ShadowMap",texture_format.r32t,width,height,texture_usage.depth_stencil_target,texture_usage.depth_stencil_target,false)

	shadow_pass:execute(shadow_pass_function)
--add_pass(shadow_pass)

local lighting_pass = RenderPass.new()
	lighting_pass:set_name("LightingPass")
	lighting_pass:set_type(render_pass_type.graphics)

	--Inputs Resources
	lighting_pass:read_texture("PositionalData",texture_usage.pixel_shader_resource)
	lighting_pass:read_texture("NormalData",texture_usage.pixel_shader_resource)
	lighting_pass:read_texture("ColorData",texture_usage.pixel_shader_resource)
	lighting_pass:read_texture("ArmData",texture_usage.pixel_shader_resource)
	lighting_pass:read_buffer("LightCompBuffer", buffer_usage.pixel_shader_resource)

	--Outputs Resources
	lighting_pass:create_texture("LightPassMap",texture_format.rgba16,width,height,texture_usage.present,texture_usage.render_target,true)


	lighting_pass:execute(lighting_pass_function)
add_pass(lighting_pass)

local skybox_pass = RenderPass.new()
	skybox_pass:set_name("SkyboxPass")
	skybox_pass:set_type(render_pass_type.graphics)

	--Input Resources
	skybox_pass:create_input_buffer("SkyBoxCBuffer",256,256,buffer_usage.constant,true,buffer_usage.constant)
	skybox_pass:read_texture("DepthPrepassBuffer",texture_usage.depth_stencil_target)
	skybox_pass:read_texture("LightPassMap",texture_usage.render_target)

	--Output Resources

	skybox_pass:execute(skybox_pass_function)
add_pass(skybox_pass)

--In this pass we render to an offscreen buffer then blit into the swap chain buffer.
--Present is only called when the editor is toggled off.
local present_pass = RenderPass.new()
	present_pass:set_name("PresentPass")
	present_pass:set_type(render_pass_type.graphics)
	present_pass:set_viewport(width, height)

	--Input Resources
	present_pass:read_buffer("SkyBoxCBuffer",buffer_usage.constant)
	present_pass:read_texture("LightPassMap",texture_usage.pixel_shader_resource)

	--Ouput Resources 
	present_pass:create_texture("OffscreenTexture",texture_format.rgba8_unorm,width,height,texture_usage.present,texture_usage.render_target,true)


	present_pass:execute(present_pass_function)
add_pass(present_pass)