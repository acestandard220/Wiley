#pragma once

//Created on 12/4/2025 at ~11:45

#include "Resource.h"
#include "DirectXMath.h"

namespace Wiley {
	
	enum class TextureChannel {
		R, G, B, A
	};
	
	/*
		mapIndex is an index into a pool of textures.
		Value is a scale value that is multiplied with the map value sampled in the shader. 
		    The UI/Editor will give an option to reset it to a value that will have no effect on your specified map.
		valueChannel is an index used in the shader to pick the right channel to use for scalar material properties such as roughness, ao and metallic.
		    This additions allows for the use of ORM maps. Properties can all reference the same texture but simply read from different channels.
	*/

	/// <summary>
	/// MaterialData struct is stored in a material data pool seperate from a parent resource.
	/// The separation is to allow for only GPU essential data to exist in on place so that only the needed data is sent to the GPU.
	/// </summary>
	struct MaterialData {

		/// <summary>
		/// Defines the diffuse/base/albedo color of the mesh/submesh/face.
		/// </summary>
		struct AlbedoProperty {
			UINT mapIndex = 0;
			DirectX::XMFLOAT4 value = { 1.0f, 1.0f, 1.0f, 1.0f };
		}albedo;

		/// <summary>
		/// Defines the normal properties of the of the mesh/submesh/face.
		/// Normal Strength of 0.0f = no map influence || 1.0f = full map influence
		/// </summary>
		struct NormalProperty {
			UINT mapIndex = 0;
			FLOAT strength = 1.0f;
		}normal;

		/// <summary>
		/// 
		/// </summary>
		struct AmbientOcclusionProperty {
			UINT mapIndex = 0;
			FLOAT value = 1.0f;
			TextureChannel valueChannel = TextureChannel::R;
		}ambientOcclusion;

		struct MetallicProperty {
			UINT mapIndex = 0;
			FLOAT value = 0.0f;
			TextureChannel valueChannel = TextureChannel::R;
		}metallic;

		struct RoughnessProperty {
			UINT mapIndex = 0;
			FLOAT value = 1.0f;
			TextureChannel valueChannel = TextureChannel::R;
		}roughness;

		DirectX::XMFLOAT2 mapScale = { 1.0f,1.0f };
	};

	struct Material : public Resource {
		MaterialData* dataPtr;

		/// <summary>
		/// These are the UUIDs to the image textures that are being used for the different properties.
		/// mapIndex is queried from them.
		/// </summary>
		UUID albedoMap;
		UUID normalMap;
		UUID metaillicMap;
		UUID roughnessMap;
		UUID ambientOcclusionMap;

		Material()
			:albedoMap(0), normalMap(0), metaillicMap(0), roughnessMap(0),
			ambientOcclusionMap(0), dataPtr(nullptr)
		{

		}
	};
}