#pragma once
#include "../Core/defines.h"
#include "../Core/Utils.h"
#include "../Core/UUID.h"
#include "../Core/FileSpace.h"


namespace Wiley
{
	enum class FileExtension {
		OBJ, GLTF, GLB, MESH,
		PNG, JPEG, BMP, TGA,
		JPG = JPEG, TOML, MTL, EXR, HDR,
		Unknown
	};

	enum class ResourceType {
		Mesh,
		Material,
		ImageTexture,
		EnvironmentMap,
		Unknown
	};

	enum class ResourceState {
		NotOnDisk,  //Not yet saved to disk
		ModOfDisk,  //Loaded from disk but has been modified
		SavedOnDisk //Engine Version == Disk Version
	};

	class ResourceCache;
	class Resource 
	{
		public:
			using Ref = std::shared_ptr<Resource>;
			using Weak = std::weak_ptr<Resource>;
			using Ptr = std::unique_ptr<Resource>;

			Resource()
				:type(ResourceType::Unknown), id(WILEY_INVALID_UUID), refCount(0), state(ResourceState::NotOnDisk)
			{

			}
			
			WILEY_NODISCARD const std::string& GetName()const { return name; }
			WILEY_NODISCARD ResourceType GetType()const { return type; }
			WILEY_NODISCARD UUID GetUUID()const { return id; }

			WILEY_NODISCARD UINT GetRefCount()const { return refCount; }
			WILEY_NODISCARD ResourceState GetState()const { return state; }


			static std::string GetResourceString(ResourceType type) {
				switch (type) {
					case ResourceType::Mesh:
						return "Mesh";
					case ResourceType::Material:
						return "Material";
					case ResourceType::ImageTexture:
						return "ImageTexture";
					case ResourceType::EnvironmentMap:
						return "EnvironmentMap";
					default: [[fallthrough]];
					case ResourceType::Unknown:
						return "Unknown";
				}
			}

			static FileExtension GetStringFileExtension(std::string_view extensionString) {
				if (extensionString == ".obj")
					return FileExtension::OBJ;
				else if (extensionString == ".gltf")
					return FileExtension::GLTF;
				else if (extensionString == ".glb")
					return FileExtension::GLB;
				else if (extensionString == ".mesh")
					return FileExtension::MESH;
				else if (extensionString == ".png")
					return FileExtension::PNG;
				else if (extensionString == ".jpg" || extensionString == ".jpeg")
					return FileExtension::JPEG;
				else if (extensionString == ".bmp")
					return FileExtension::BMP;
				else if (extensionString == ".tga")
					return FileExtension::TGA;
				else if (extensionString == ".toml")
					return FileExtension::TOML;
				else if (extensionString == ".hdr")
					return FileExtension::HDR;
				else if (extensionString == ".exr")
					return FileExtension::EXR;
				else {
					std::cout << "File Format is not supported." << std::endl;
					return FileExtension::Unknown;
				}
			}
			
		private:
			friend class ResourceCache;

			ResourceType type;
			std::string name;
			filespace::filepath path;
			UUID id;

			UINT refCount;
			ResourceState state;
	};

	template<typename T>
	concept IsResourceType = std::derived_from<T, Resource>;
}
