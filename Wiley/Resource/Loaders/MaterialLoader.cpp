#include "../ResourceLoader.h"
#include "../ResourceCache.h"

#include "mtl_parser.h"

#include "toml.hpp"

#include <cstdlib>
#include <fstream>

namespace Wiley {

    static std::string GetChannelString(TextureChannel channel) {
        switch (channel) {
        case TextureChannel::R: return "R";
        case TextureChannel::G: return "G";
        case TextureChannel::B: return "B";
        case TextureChannel::A: return "A";
        }
    }

    static TextureChannel GetStringChannel(const std::string& channel) {
        if (channel == "R")return TextureChannel::R;
        else if (channel == "G")return TextureChannel::G;
        else if (channel == "B")return TextureChannel::B;
        else if (channel == "A")return TextureChannel::A;
    }




    template<IsResourceType ResourceClass>
    Resource::Ref LoadResourceDep(toml::table& node, const std::string resourceName, ResourceLoadDesc& loadDesc, ResourceCache* resourceCache) {
        auto depUUIDs = node["dependencies.uuid"];
        auto depPaths = node["dependencies.path"];

        auto& UUIDString = depUUIDs[resourceName].as_string()->get();
        auto& Path = depPaths[resourceName].as_string()->get();
        auto uuid = WILEY_UUID_FROM_STRING(UUIDString);

        loadDesc.id = uuid;
        loadDesc.extension = Resource::GetStringFileExtension(filespace::Extension(Path));

        return resourceCache->LoadResource<ResourceClass>(Path, loadDesc);
    }

    Resource::Ref MaterialLoader::LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        switch (loadDesc.extension) {
        case FileExtension::TOML:
            return LoadTOMLFromFile(path, loadDesc);
        case FileExtension::MTL:
            return LoadMTLFromFile(path, loadDesc);
        }
    }

    Resource::Ref MaterialLoader::LoadMTLFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        std::shared_ptr<Material> materialRef = std::make_shared<Material>();
        MemoryBlock<MaterialData> memoryBlk = resourceCache->materialDataPool->Allocate(1);
        materialRef->dataPtr = memoryBlk.data();

        std::ifstream file(path.string().c_str());
        if (!file.is_open())
        {
            std::cout << "Failed to open MTL File." << std::endl;
            return nullptr;
        }

        {
            process_status proc_status;
            auto material_files = load_mtl_material(path.string().c_str(), proc_status);

            for (const auto& mtlFile : material_files) {
                mtlFile.name;
                ResourceLoadDesc albedoLoadDesc = { .desc = {.imageTextureDesc = {.type = MapType::Albedo}} };
                ResourceLoadDesc normalLoadDesc = { .desc = {.imageTextureDesc = {.type = MapType::Normal}} };
                ResourceLoadDesc aoLoadDesc = { .desc = {.imageTextureDesc = {.type = MapType::AO}} };
                ResourceLoadDesc metallicLoadDesc = { .desc = {.imageTextureDesc = {.type = MapType::Metalloic}} };
                ResourceLoadDesc roughnessLoadDesc = { .desc = {.imageTextureDesc = {.type = MapType::Roughness}} };

                ImageTexture* albedoResource = static_cast<ImageTexture*>(resourceCache->LoadResource<ImageTexture>(mtlFile.paths.albedo, albedoLoadDesc).get());
                ImageTexture* normalResource = static_cast<ImageTexture*>(resourceCache->LoadResource<ImageTexture>(mtlFile.paths.normal, normalLoadDesc).get());
                ImageTexture* aoResource = static_cast<ImageTexture*>(resourceCache->LoadResource<ImageTexture>(mtlFile.paths.ao, aoLoadDesc).get());
                ImageTexture* metalloicResource = static_cast<ImageTexture*>(resourceCache->LoadResource<ImageTexture>(mtlFile.paths.metallic, metallicLoadDesc).get());
                ImageTexture* roughnessResource = static_cast<ImageTexture*>(resourceCache->LoadResource<ImageTexture>(mtlFile.paths.roughness, roughnessLoadDesc).get());

                auto mtlDataPtr = materialRef->dataPtr;
                mtlDataPtr->albedo.mapIndex = albedoResource->srvIndex;
                mtlDataPtr->albedo.value = mtlFile.values.albedo;

                mtlDataPtr->ambientOcclusion.mapIndex = aoResource->srvIndex;
                mtlDataPtr->ambientOcclusion.value = mtlFile.values.ao;
                mtlDataPtr->ambientOcclusion.valueChannel;

                mtlDataPtr->normal.mapIndex = normalResource->srvIndex;
                mtlDataPtr->normal.strength = mtlFile.values.normal;

                mtlDataPtr->roughness.mapIndex = roughnessResource->srvIndex;
                mtlDataPtr->roughness.value = mtlFile.values.roughness;
                mtlDataPtr->roughness.valueChannel;

                mtlDataPtr->metallic.mapIndex = metalloicResource->srvIndex;
                mtlDataPtr->metallic.valueChannel;
                mtlDataPtr->metallic.value = mtlFile.values.metallic;

                mtlDataPtr->mapScale = mtlFile.scale;


                materialRef->albedoMap = albedoResource->GetUUID();
                materialRef->normalMap = normalResource->GetUUID();
                materialRef->ambientOcclusionMap = aoResource->GetUUID();
                materialRef->roughnessMap = roughnessResource->GetUUID();
                materialRef->metaillicMap = metalloicResource->GetUUID();

                std::cout << "Only the first material in the mtl data base will be extracted.\n";
                break;

            }
        }

        return materialRef;
    }

    Resource::Ref MaterialLoader::LoadTOMLFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        std::shared_ptr<Material> materialRef = std::make_shared<Material>();
        MemoryBlock<MaterialData> memoryBlk = resourceCache->materialDataPool->Allocate(1);
        materialRef->dataPtr = memoryBlk.data();

        toml::table node = toml::parse_file(path.string());

        ResourceLoadDesc imageLoadDesc{};

        imageLoadDesc.desc.imageTextureDesc.type = MapType::Albedo;
        ImageTexture* albedoTexture = static_cast<ImageTexture*>(LoadResourceDep<ImageTexture>(node, "Albedo", imageLoadDesc, resourceCache).get());
        if (!albedoTexture) {
            std::cout << "Failed to load dependency image texture." << std::endl;
        }

        imageLoadDesc.desc.imageTextureDesc.type = MapType::Normal;
        ImageTexture* normalTexture = static_cast<ImageTexture*>(LoadResourceDep<ImageTexture>(node, "Normal", imageLoadDesc, resourceCache).get());
        if (!normalTexture) {
            std::cout << "Failed to load dependency image texture." << std::endl;
        }

        imageLoadDesc.desc.imageTextureDesc.type = MapType::Metalloic;
        ImageTexture* metallicTexture = static_cast<ImageTexture*>(LoadResourceDep<ImageTexture>(node, "Metallic", imageLoadDesc, resourceCache).get());
        if (!metallicTexture) {
            std::cout << "Failed to load dependency image texture." << std::endl;
        }

        imageLoadDesc.desc.imageTextureDesc.type = MapType::Roughness;
        ImageTexture* roughnessTexture = static_cast<ImageTexture*>(LoadResourceDep<ImageTexture>(node, "Roughness", imageLoadDesc, resourceCache).get());
        if (!roughnessTexture) {
            std::cout << "Failed to load dependency image texture." << std::endl;
        }

        imageLoadDesc.desc.imageTextureDesc.type = MapType::AO;
        ImageTexture* aoTexture = static_cast<ImageTexture*>(LoadResourceDep<ImageTexture>(node, "AmbientOcclusion", imageLoadDesc, resourceCache).get());
        if (!aoTexture) {
            std::cout << "Failed to load dependency image texture." << std::endl;
        }


        {
            auto uuidStringValue = node["UUID"];
            auto uuid = WILEY_UUID_FROM_STRING(uuidStringValue.as_string()->get());
            loadDesc.id = uuid;

            materialRef->albedoMap = albedoTexture->GetUUID();
            materialRef->normalMap = normalTexture->GetUUID();
            materialRef->ambientOcclusionMap = aoTexture->GetUUID();
            materialRef->metaillicMap = metallicTexture->GetUUID();
            materialRef->roughnessMap = roughnessTexture->GetUUID();
        }

        {
            MaterialData* mtlData = materialRef->dataPtr;

            auto albedoPropertiesTable = node["properties.albedo"];
            auto albedoPropertiesValueArr = albedoPropertiesTable["value"].as_array();
            mtlData->albedo = {
                .mapIndex = (albedoTexture) ? albedoTexture->srvIndex : 0,
                .value = {
                    static_cast<float>((*albedoPropertiesValueArr)[0].value_or(1.0f)),
                    static_cast<float>((*albedoPropertiesValueArr)[1].value_or(1.0f)),
                    static_cast<float>((*albedoPropertiesValueArr)[2].value_or(1.0f)),
                    static_cast<float>((*albedoPropertiesValueArr)[3].value_or(1.0f))
                }
            };

            auto normalPropertiesTable = node["properties.normal"];
            auto normalPropertiesStrengthValue = normalPropertiesTable["strength"];
            mtlData->normal = {
                .mapIndex = (normalTexture) ? normalTexture->srvIndex : 0,
                .strength = static_cast<float>(normalPropertiesStrengthValue.value_or(1.0f))
            };

            auto aoPropertyTable = node["properties.ambient_occlusion"];
            auto aoScalarValue = aoPropertyTable["value"];
            auto aoChannelValue = aoPropertyTable["channel"];
            mtlData->ambientOcclusion = {
                .mapIndex = (aoTexture) ? aoTexture->srvIndex : 0,
                .value = static_cast<float>(aoScalarValue.value_or(1.0f)),
                .valueChannel = GetStringChannel(aoChannelValue.as_string()->get())
            };

            auto metallicPropertyTable = node["properties.metallic"];
            auto metallicScalarValue = metallicPropertyTable["value"];
            auto metallicChannelValue = metallicPropertyTable["channel"];
            mtlData->metallic = {
                .mapIndex = (metallicTexture) ? metallicTexture->srvIndex : 0,
                .value = static_cast<float>(metallicScalarValue.value_or(0.0f)),
                .valueChannel = GetStringChannel(metallicChannelValue.as_string()->get())
            };

            auto roughnessPropertyTable = node["properties.roughness"];
            auto roughnessScalarValue = roughnessPropertyTable["value"];
            auto roughnessChannelValue = roughnessPropertyTable["channel"];
            mtlData->roughness = {
                .mapIndex = (roughnessTexture) ? roughnessTexture->srvIndex : 0,
                .value = static_cast<float>(roughnessScalarValue.value_or(1.0f)),
                .valueChannel = GetStringChannel(roughnessChannelValue.as_string()->get())
            };

            auto globalProperties = node["properties.global"];
            auto mapScaleArr = globalProperties["scale"].as_array();
            mtlData->mapScale = {
                static_cast<float>((*mapScaleArr)[0].value_or(1.0f)),
                static_cast<float>((*mapScaleArr)[1].value_or(1.0f))
            };
        }
        return materialRef;
    }

    Resource::Ref MaterialLoader::CreateNew(filespace::filepath path)
    {
        std::shared_ptr<Material> materialRef = std::make_shared<Material>();
        MemoryBlock<MaterialData> memoryBlk = resourceCache->materialDataPool->Allocate(1);
        materialRef->dataPtr = (MaterialData*)memoryBlk.data();

        MaterialData* materialData = materialRef->dataPtr;

        materialData->albedo.value = { 1.0f,1.0f,1.0f,1.0f };
        materialData->normal.strength = 1.0f;
        materialData->metallic.value = 1.0f;
        materialData->roughness.value = 1.0f;
        materialData->ambientOcclusion.value = 1.0f;
        materialData->mapScale = { 1.0f,1.0f };

        return materialRef;
    }


    void MaterialLoader::SaveToFile(filespace::filepath path, Material* material)
    {
        auto node = toml::table{
            { "UUID", WILEY_UUID_STRING(material->GetUUID())},
            { "Type", Resource::GetResourceString(ResourceType::Material)},
            { "Name", resourceCache->GetResourceName<Material>(material->GetUUID())},

            { "dependencies.uuid", toml::table{
                 {"Albedo", WILEY_UUID_STRING(material->albedoMap)},
                 {"Normal", WILEY_UUID_STRING(material->normalMap)},
                 {"AmbientOcclusion", WILEY_UUID_STRING(material->ambientOcclusionMap)},
                 {"Roughness", WILEY_UUID_STRING(material->roughnessMap)},
                 {"Metallic", WILEY_UUID_STRING(material->metaillicMap)},
            }},


            { "dependencies.path", toml::table{
                {"Albedo", resourceCache->GetResourcePath<ImageTexture>(material->albedoMap).string()},
                {"Normal", resourceCache->GetResourcePath<ImageTexture>(material->normalMap).string()},
                {"AmbientOcclusion", resourceCache->GetResourcePath<ImageTexture>(material->ambientOcclusionMap).string()},
                {"Roughness", resourceCache->GetResourcePath<ImageTexture>(material->roughnessMap).string()},
                {"Metallic", resourceCache->GetResourcePath<ImageTexture>(material->metaillicMap).string()}
            }},

            { "properties.albedo", toml::table{
                { "value",toml::array{ material->dataPtr->albedo.value.x,
                                       material->dataPtr->albedo.value.y,
                                       material->dataPtr->albedo.value.z,
                                       material->dataPtr->albedo.value.w}
                }
            }},
            { "properties.normal", toml::table{
                { "value", material->dataPtr->normal.strength}
            }},
            { "properties.ambient_occlusion", toml::table{
                { "value", material->dataPtr->ambientOcclusion.value},
                { "channel", GetChannelString(material->dataPtr->ambientOcclusion.valueChannel)},
            }},
            { "properties.roughness", toml::table{
                { "value", material->dataPtr->roughness.value},
                { "channel", GetChannelString(material->dataPtr->roughness.valueChannel)}
            }},
            { "properties.metallic", toml::table{
                { "value", material->dataPtr->metallic.value},
                { "channel", GetChannelString(material->dataPtr->metallic.valueChannel)}
            }},

            { "properties.global", toml::table{
                {"scale", toml::array(material->dataPtr->mapScale.x,
                                    material->dataPtr->mapScale.y)
                }
            }}
        };

        std::ofstream file(path);
        if (!file.is_open()) {
            std::cout << "Failed to open file for write." << std::endl;
            return;
        }

        file << node;

        file.close();
    }

}