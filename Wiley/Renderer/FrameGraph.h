#pragma once

//Created on 11/10/2025 16:10

#include "../RHI/GraphicsPipeline.h"
#include "../RHI/Texture.h"
#include "../RHI/RenderContext.h"

#include "sol/sol.hpp"
#include "Tracy/tracy/Tracy.hpp"

#include <memory>
#include <functional>
#include <queue>
#include <string>
#include <unordered_set>
#include <variant>
#include <string_view>
#include <ranges>

template<typename...Types>
using Variant = std::variant<Types...>;

namespace Renderer3D
{
	struct TextureResourceDesc
	{
		RHI::TextureFormat format;
		uint32_t width, height;
		RHI::TextureUsage usage;

		bool operator==(const TextureResourceDesc& other)
		{
			return (format == other.format) && (width == other.width) &&
				(height == other.height) && (usage == other.usage);
		}
	};

	struct BufferResourceDesc {
		UINT64 size;
		UINT64 stride;
		RHI::BufferFormat format;
		RHI::BufferUsage usage;
		bool persistent;
	};

	enum class ResourcePool {
		Transient, Persistent
	};

	enum class RenderPassSemantic
	{
		//Graphics
		Wireframe,
		Geometry,
		ShadowMapPass,
		PresentPass,
		DepthPrepass,
		ClusterHeapMapPass,
		LightingPass,
		SkyboxPass,

		//Compute		
		GenClusterPass,
		ClusterCullPass,
		CompactClusterPass,
		ClusterAssignment
	};

	enum class RenderPassType {
		Graphics, Copy, Compute, Unknown
	};

	
	enum class PassResourceType {
		Texture, Buffer, Unknown
	};

	struct ResourceHandle
	{
		UINT id = UINT_MAX;
		Variant<RHI::TextureUsage, RHI::BufferUsage> usage;
		Variant<RHI::TextureUsage, RHI::BufferUsage> creationState;

		bool IsValid()const { return id != UINT_MAX; }
	};

	struct RenderPass
	{
		std::string name;

		std::vector<ResourceHandle> inputs;
		std::vector<ResourceHandle> outputs;

		std::function<void(RenderPass&)> execute;
		std::function<bool()> condition;

		DirectX::XMFLOAT3 passDimensions;
		RenderPassType passType = RenderPassType::Unknown;
		UINT waitValue;

		RenderPass()
			:passDimensions{ 0.0f,0.0f,0.0f }, waitValue(0)
		{

		}
	};

	struct RenderPassResource 
	{
		Variant<RHI::Texture::Ref, RHI::Buffer::Ref>resource;
		PassResourceType type = PassResourceType::Unknown;
		bool isScreenSizeDependent = false;
	};

	//This class decides what resources get created and their lifetime during a frame.
	//Input Resource slot allocation per pass are automatic. Input resources are given slot IDs sequentially based on the order they defined.
	class FrameGraph
	{
		public:
			using Ref = std::shared_ptr<FrameGraph>;
			using Ptr = std::unique_ptr<FrameGraph>;

			FrameGraph() = default;
			FrameGraph(RHI::RenderContext::Ref rctx);
			~FrameGraph() = default;

			ResourceHandle CreateTextureResource(const std::string& name, const TextureResourceDesc& resourceDesc, RHI::TextureUsage usage, bool isScreenSizeDependent = false);
			ResourceHandle CreateBufferResource(const std::string& name, const BufferResourceDesc& resourceDesc, RHI::BufferUsage usage);
			ResourceHandle ImportTextureResource(const std::string& name, RHI::Texture::Ref texture, RHI::TextureUsage usage);
			ResourceHandle ImportBufferResource(const std::string& name, RHI::Buffer::Ref buffer, RHI::BufferUsage usage);

			ResourceHandle ReadTextureResource(const std::string& name,RHI::TextureUsage usage);
			ResourceHandle ReadBufferResource(const std::string& name, RHI::BufferUsage usage);

			ResourceHandle WriteTextureResource(const std::string& name,RHI::TextureUsage usage);
			ResourceHandle WriteBufferResource(const std::string& name, RHI::BufferUsage usage);

			RenderPassResource GetResource(const std::string& resourceName);

			//Requires client to manually call std::get<T>(std::variant)
			RenderPassResource GetResource(const ResourceHandle& handle);
			RenderPassResource GetResource(std::vector<ResourceHandle> const& res, int accessSlot);

			//Type-Safe Version. Variants are sorted in these versions.
			RHI::Texture::Ref GetInputTextureResource(RenderPass& pass, int accessSlot);
			RHI::Buffer::Ref GetInputBufferResource(RenderPass& pass, int accessSlot);

			RHI::Texture::Ref GetOutputTextureResource(RenderPass& pass, int accessSlot);
			RHI::Buffer::Ref GetOutputBufferResource(RenderPass& pass, int accessSlot);
			
			void TransistionInputTextures(RenderPass& pass);
			void TransitionOutputTextures(RenderPass& pass);
			void TransitionInputBuffers(RenderPass& pass);

			void TransitionInputTextureToCreationState(RenderPass& pass);
			void TransitionOutputTextureToCreationState(RenderPass& pass);

			UINT GetPassWaitValue(const std::string& name);

			void AddPass(const RenderPass& pass);
			void Compile();
			void Execute()const;			

			void OnResize(std::uint32_t width, std::uint32_t height);

			std::vector<std::string_view> GetPassOrder();
		private:
			std::vector<RenderPass> passes;
			std::vector<RenderPass*> sortedPasses;

			std::unordered_map<std::string, uint32_t> strDesc;
			std::unordered_map<std::string, ResourcePool> poolMap;

			std::vector<RenderPassResource> resources;

			RHI::RenderContext::Ref rctx;
	};
}

namespace std {
	template<>
	struct hash<Renderer3D::TextureResourceDesc> {
		size_t operator()(const Renderer3D::TextureResourceDesc& desc) const {
			return hash<uint32_t>()(static_cast<uint32_t>(desc.format)) ^
				(hash<uint32_t>()(desc.width) << 1) ^
				(hash<uint32_t>()(desc.height) << 2) ^
				(hash<uint32_t>()(static_cast<uint32_t>(desc.usage)) << 3);
		}
	};
}