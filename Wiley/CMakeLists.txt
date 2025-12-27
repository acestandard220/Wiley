set(PROJECT_NAME Wiley)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "framegraph.lua"
    "script.lua"
)
source_group("" FILES ${no_group_source_files})

set(Header_Files
    "Core/Allocator.h"
    "Core/defines.h"
    "Core/FileSpace.h"
    "Core/Input.h"
    "Core/ScriptEngine.h"
    "Core/ThreadPool.h"
    "Core/Utils.h"
    "Core/UUID.h"
    "Core/Window.h"
    "ext/DX12 Helper/d3dx12.h"
    "ext/DX12 Helper/d3dx12_barriers.h"
    "ext/DX12 Helper/d3dx12_check_feature_support.h"
    "ext/DX12 Helper/d3dx12_core.h"
    "ext/DX12 Helper/d3dx12_default.h"
    "ext/DX12 Helper/d3dx12_pipeline_state_stream.h"
    "ext/DX12 Helper/d3dx12_property_format_table.h"
    "ext/DX12 Helper/d3dx12_render_pass.h"
    "ext/DX12 Helper/d3dx12_resource_helpers.h"
    "ext/DX12 Helper/d3dx12_root_signature.h"
    "ext/DX12 Helper/d3dx12_state_object.h"
    "ext/DX12 Helper/DDSTextureLoader.h"
    "ext/DX12 Helper/DXSampleHelper.h"
    "ext/DX12 Helper/MathHelper.h"
    "ext/entt.hpp"
    "ext/ImGui/imconfig.h"
    "ext/ImGui/imgui.h"
    "ext/ImGui/imgui_impl_dx12.h"
    "ext/ImGui/imgui_impl_win32.h"
    "ext/ImGui/imgui_internal.h"
    "ext/ImGui/imstb_rectpack.h"
    "ext/ImGui/imstb_textedit.h"
    "ext/ImGui/imstb_truetype.h"
    "ext/OpTick/src/optick.config.h"
    "ext/OpTick/src/optick.h"
    "ext/OpTick/src/optick_capi.h"
    "ext/OpTick/src/optick_common.h"
    "ext/OpTick/src/optick_core.freebsd.h"
    "ext/OpTick/src/optick_core.h"
    "ext/OpTick/src/optick_core.linux.h"
    "ext/OpTick/src/optick_core.macos.h"
    "ext/OpTick/src/optick_core.platform.h"
    "ext/OpTick/src/optick_core.win.h"
    "ext/OpTick/src/optick_gpu.h"
    "ext/OpTick/src/optick_memory.h"
    "ext/OpTick/src/optick_message.h"
    "ext/OpTick/src/optick_miniz.h"
    "ext/OpTick/src/optick_serialization.h"
    "ext/OpTick/src/optick_server.h"
    "ext/sol.hpp"
    "ext/sol/abort.hpp"
    "ext/sol/as_args.hpp"
    "ext/sol/as_returns.hpp"
    "ext/sol/assert.hpp"
    "ext/sol/base_traits.hpp"
    "ext/sol/bind_traits.hpp"
    "ext/sol/bytecode.hpp"
    "ext/sol/call.hpp"
    "ext/sol/compatibility.hpp"
    "ext/sol/compatibility/compat-5.3.c.h"
    "ext/sol/compatibility/compat-5.3.h"
    "ext/sol/compatibility/compat-5.4.h"
    "ext/sol/compatibility/lua_version.hpp"
    "ext/sol/config.hpp"
    "ext/sol/coroutine.hpp"
    "ext/sol/debug.hpp"
    "ext/sol/demangle.hpp"
    "ext/sol/deprecate.hpp"
    "ext/sol/detail/build_version.hpp"
    "ext/sol/dump_handler.hpp"
    "ext/sol/ebco.hpp"
    "ext/sol/environment.hpp"
    "ext/sol/epilogue.hpp"
    "ext/sol/error.hpp"
    "ext/sol/error_handler.hpp"
    "ext/sol/forward.hpp"
    "ext/sol/forward_as.hpp"
    "ext/sol/forward_detail.hpp"
    "ext/sol/function.hpp"
    "ext/sol/function_result.hpp"
    "ext/sol/function_types.hpp"
    "ext/sol/function_types_core.hpp"
    "ext/sol/function_types_overloaded.hpp"
    "ext/sol/function_types_stateful.hpp"
    "ext/sol/function_types_stateless.hpp"
    "ext/sol/function_types_templated.hpp"
    "ext/sol/in_place.hpp"
    "ext/sol/inheritance.hpp"
    "ext/sol/lapi.h"
    "ext/sol/lauxlib.h"
    "ext/sol/lcode.h"
    "ext/sol/lctype.h"
    "ext/sol/ldebug.h"
    "ext/sol/ldo.h"
    "ext/sol/lfunc.h"
    "ext/sol/lgc.h"
    "ext/sol/ljumptab.h"
    "ext/sol/llex.h"
    "ext/sol/llimits.h"
    "ext/sol/lmem.h"
    "ext/sol/load_result.hpp"
    "ext/sol/lobject.h"
    "ext/sol/lopcodes.h"
    "ext/sol/lopnames.h"
    "ext/sol/lparser.h"
    "ext/sol/lprefix.h"
    "ext/sol/lstate.h"
    "ext/sol/lstring.h"
    "ext/sol/ltable.h"
    "ext/sol/ltm.h"
    "ext/sol/lua.h"
    "ext/sol/lua.hpp"
    "ext/sol/lua/lapi.h"
    "ext/sol/lua/lauxlib.h"
    "ext/sol/lua/lcode.h"
    "ext/sol/lua/lctype.h"
    "ext/sol/lua/ldebug.h"
    "ext/sol/lua/ldo.h"
    "ext/sol/lua/lfunc.h"
    "ext/sol/lua/lgc.h"
    "ext/sol/lua/ljumptab.h"
    "ext/sol/lua/llex.h"
    "ext/sol/lua/llimits.h"
    "ext/sol/lua/lmem.h"
    "ext/sol/lua/lobject.h"
    "ext/sol/lua/lopcodes.h"
    "ext/sol/lua/lopnames.h"
    "ext/sol/lua/lparser.h"
    "ext/sol/lua/lprefix.h"
    "ext/sol/lua/lstate.h"
    "ext/sol/lua/lstring.h"
    "ext/sol/lua/ltable.h"
    "ext/sol/lua/ltm.h"
    "ext/sol/lua/lua.h"
    "ext/sol/lua/lua.hpp"
    "ext/sol/lua/luaconf.h"
    "ext/sol/lua/lualib.h"
    "ext/sol/lua/lundump.h"
    "ext/sol/lua/lvm.h"
    "ext/sol/lua/lzio.h"
    "ext/sol/lua_table.hpp"
    "ext/sol/lua_value.hpp"
    "ext/sol/luaconf.h"
    "ext/sol/lualib.h"
    "ext/sol/lundump.h"
    "ext/sol/lvm.h"
    "ext/sol/lzio.h"
    "ext/sol/make_reference.hpp"
    "ext/sol/metatable.hpp"
    "ext/sol/object.hpp"
    "ext/sol/object_base.hpp"
    "ext/sol/optional.hpp"
    "ext/sol/optional_implementation.hpp"
    "ext/sol/overload.hpp"
    "ext/sol/packaged_coroutine.hpp"
    "ext/sol/pairs_iterator.hpp"
    "ext/sol/pointer_like.hpp"
    "ext/sol/policies.hpp"
    "ext/sol/prologue.hpp"
    "ext/sol/property.hpp"
    "ext/sol/protect.hpp"
    "ext/sol/protected_function.hpp"
    "ext/sol/protected_function_result.hpp"
    "ext/sol/protected_handler.hpp"
    "ext/sol/proxy_base.hpp"
    "ext/sol/raii.hpp"
    "ext/sol/reference.hpp"
    "ext/sol/resolve.hpp"
    "ext/sol/sol.hpp"
    "ext/sol/stack.hpp"
    "ext/sol/stack/detail/pairs.hpp"
    "ext/sol/stack_check.hpp"
    "ext/sol/stack_check_get.hpp"
    "ext/sol/stack_check_get_qualified.hpp"
    "ext/sol/stack_check_get_unqualified.hpp"
    "ext/sol/stack_check_qualified.hpp"
    "ext/sol/stack_check_unqualified.hpp"
    "ext/sol/stack_core.hpp"
    "ext/sol/stack_field.hpp"
    "ext/sol/stack_get.hpp"
    "ext/sol/stack_get_qualified.hpp"
    "ext/sol/stack_get_unqualified.hpp"
    "ext/sol/stack_guard.hpp"
    "ext/sol/stack_iterator.hpp"
    "ext/sol/stack_pop.hpp"
    "ext/sol/stack_probe.hpp"
    "ext/sol/stack_proxy.hpp"
    "ext/sol/stack_proxy_base.hpp"
    "ext/sol/stack_push.hpp"
    "ext/sol/stack_reference.hpp"
    "ext/sol/state.hpp"
    "ext/sol/state_handling.hpp"
    "ext/sol/state_view.hpp"
    "ext/sol/string_view.hpp"
    "ext/sol/table.hpp"
    "ext/sol/table_core.hpp"
    "ext/sol/table_iterator.hpp"
    "ext/sol/table_proxy.hpp"
    "ext/sol/thread.hpp"
    "ext/sol/tie.hpp"
    "ext/sol/traits.hpp"
    "ext/sol/trampoline.hpp"
    "ext/sol/tuple.hpp"
    "ext/sol/types.hpp"
    "ext/sol/unicode.hpp"
    "ext/sol/unique_usertype_traits.hpp"
    "ext/sol/unreachable.hpp"
    "ext/sol/unsafe_function.hpp"
    "ext/sol/unsafe_function_result.hpp"
    "ext/sol/userdata.hpp"
    "ext/sol/usertype.hpp"
    "ext/sol/usertype_container.hpp"
    "ext/sol/usertype_container_launch.hpp"
    "ext/sol/usertype_core.hpp"
    "ext/sol/usertype_proxy.hpp"
    "ext/sol/usertype_storage.hpp"
    "ext/sol/usertype_traits.hpp"
    "ext/sol/utility/is_integer.hpp"
    "ext/sol/utility/to_string.hpp"
    "ext/sol/variadic_args.hpp"
    "ext/sol/variadic_results.hpp"
    "ext/sol/version.hpp"
    "ext/sol/wrapper.hpp"
    "ext/stb_image.h"
    "ext/stb_image_write.h"
    "ext/tiny_gltf.h"
    "ext/tiny_obj_loader.h"
    "ext/toml.hpp"
    "Renderer/FrameGraph.h"
    "Renderer/Renderer.h"
    "Resource/Geometry.h"
    "Resource/ImageTexture.h"
    "Resource/Material.h"
    "Resource/Resource.h"
    "Resource/ResourceCache.h"
    "Resource/ResourceLoader.h"
    "RHI/Buffer.h"
    "RHI/CommandList.h"
    "RHI/CommandQueue.h"
    "RHI/ComputePipeline.h"
    "RHI/DescriptorHeap.h"
    "RHI/Device.h"
    "RHI/Fence.h"
    "RHI/GraphicsPipeline.h"
    "RHI/IndirectCommandBuffer.h"
    "RHI/InputLayout.h"
    "RHI/RenderContext.h"
    "RHI/RootSignature.h"
    "RHI/Sampler.h"
    "RHI/ShaderByteCode.h"
    "RHI/SwapChain.h"
    "RHI/Texture.h"
    "Scene/Camera.h"
    "Scene/Component.h"
    "Scene/Entity.h"
    "Scene/Scene.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "Core/Allocator.cpp"
    "Core/FileSpace.cpp"
    "Core/Input.cpp"
    "Core/ScriptEngine.cpp"
    "Core/ThreadPool.cpp"
    "Core/Utils.cpp"
    "Core/UUID.cpp"
    "Core/Window.cpp"
    "ext/DX12 Helper/MathHelper.cpp"
    "ext/ImGui/imgui.cpp"
    "ext/ImGui/imgui_demo.cpp"
    "ext/ImGui/imgui_draw.cpp"
    "ext/ImGui/imgui_impl_dx12.cpp"
    "ext/ImGui/imgui_impl_win32.cpp"
    "ext/ImGui/imgui_tables.cpp"
    "ext/ImGui/imgui_widgets.cpp"
    "ext/OpTick/src/optick_capi.cpp"
    "ext/OpTick/src/optick_core.cpp"
    "ext/OpTick/src/optick_gpu.cpp"
    "ext/OpTick/src/optick_gpu.d3d12.cpp"
    "ext/OpTick/src/optick_message.cpp"
    "ext/OpTick/src/optick_miniz.cpp"
    "ext/OpTick/src/optick_serialization.cpp"
    "ext/OpTick/src/optick_server.cpp"
    "ext/sol/lapi.c"
    "ext/sol/lauxlib.c"
    "ext/sol/lbaselib.c"
    "ext/sol/lcode.c"
    "ext/sol/lcorolib.c"
    "ext/sol/lctype.c"
    "ext/sol/ldblib.c"
    "ext/sol/ldebug.c"
    "ext/sol/ldo.c"
    "ext/sol/ldump.c"
    "ext/sol/lfunc.c"
    "ext/sol/lgc.c"
    "ext/sol/linit.c"
    "ext/sol/liolib.c"
    "ext/sol/llex.c"
    "ext/sol/lmathlib.c"
    "ext/sol/lmem.c"
    "ext/sol/loadlib.c"
    "ext/sol/lobject.c"
    "ext/sol/lopcodes.c"
    "ext/sol/loslib.c"
    "ext/sol/lparser.c"
    "ext/sol/lstate.c"
    "ext/sol/lstring.c"
    "ext/sol/lstrlib.c"
    "ext/sol/ltable.c"
    "ext/sol/ltablib.c"
    "ext/sol/ltm.c"
    "ext/sol/lua/lapi.c"
    "ext/sol/lua/lauxlib.c"
    "ext/sol/lua/lbaselib.c"
    "ext/sol/lua/lcode.c"
    "ext/sol/lua/lcorolib.c"
    "ext/sol/lua/lctype.c"
    "ext/sol/lua/ldblib.c"
    "ext/sol/lua/ldebug.c"
    "ext/sol/lua/ldo.c"
    "ext/sol/lua/ldump.c"
    "ext/sol/lua/lfunc.c"
    "ext/sol/lua/lgc.c"
    "ext/sol/lua/linit.c"
    "ext/sol/lua/liolib.c"
    "ext/sol/lua/llex.c"
    "ext/sol/lua/lmathlib.c"
    "ext/sol/lua/lmem.c"
    "ext/sol/lua/loadlib.c"
    "ext/sol/lua/lobject.c"
    "ext/sol/lua/lopcodes.c"
    "ext/sol/lua/loslib.c"
    "ext/sol/lua/lparser.c"
    "ext/sol/lua/lstate.c"
    "ext/sol/lua/lstring.c"
    "ext/sol/lua/lstrlib.c"
    "ext/sol/lua/ltable.c"
    "ext/sol/lua/ltablib.c"
    "ext/sol/lua/ltm.c"
    "ext/sol/lua/luac.c"
    "ext/sol/lua/lundump.c"
    "ext/sol/lua/lutf8lib.c"
    "ext/sol/lua/lvm.c"
    "ext/sol/lua/lzio.c"
    "ext/sol/lundump.c"
    "ext/sol/lutf8lib.c"
    "ext/sol/lvm.c"
    "ext/sol/lzio.c"
    "ext/stb.cpp"
    "ext/tiny_obj_loader.cpp"
    "Renderer/FrameGraph.cpp"
    "Renderer/Passes/GeometryPass.cpp"
    "Renderer/Passes/LightCullPass.cpp"
    "Renderer/Passes/PipelineStates.cpp"
    "Renderer/Renderer.cpp"
    "Renderer/RenderPasses.cpp"
    "Renderer/RenderScript.cpp"
    "Resource/Geometry.cpp"
    "Resource/ResourceCache.cpp"
    "Resource/ResourceLoader.cpp"
    "RHI/Buffer.cpp"
    "RHI/CommandList.cpp"
    "RHI/CommandQueue.cpp"
    "RHI/ComputePipeline.cpp"
    "RHI/DescriptorHeap.cpp"
    "RHI/Device.cpp"
    "RHI/Fence.cpp"
    "RHI/GraphicsPipeline.cpp"
    "RHI/IndirectCommandBuffer.cpp"
    "RHI/RenderContext.cpp"
    "RHI/RootSignature.cpp"
    "RHI/Sampler.cpp"
    "RHI/ShaderByteCode.cpp"
    "RHI/SwapChain.cpp"
    "RHI/Texture.cpp"
    "Scene/Camera.cpp"
    "Scene/Scene.cpp"
    "Wiley.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Wiley)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<$<CONFIG:Debug>:"
            "${CMAKE_CURRENT_SOURCE_DIR}/ext/sol"
        ">"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../DevTools/DX12 Helper;"
        "${CMAKE_CURRENT_SOURCE_DIR}/ext/OpTick/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/ext/ImGui;"
        "${CMAKE_CURRENT_SOURCE_DIR}/ext"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_CRT_SECURE_NO_WARNINGS;"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ext/dxlibs"
    )
endif()

