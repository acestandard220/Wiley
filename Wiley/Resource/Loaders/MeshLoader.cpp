#define SHOULD_INCLUDE_ASSIMP

#include "../ResourceLoader.h"
#include "../ResourceCache.h"

#include "tiny_obj_loader.h"
#include "meshoptimizer/src/meshoptimizer.h"



#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include <cstdlib>
#include <fstream>

namespace Wiley {

    /// <todo>
    ///     [meshoptimizer]
    ///     Clean Up(Duplication) #
    ///     Vertex Cache Optimization #
    ///     Overdraw Optimization #
    ///     Vertex Fetch Optimization #
    ///     LOD system
    /// </todo>

    struct _MeshFileHead {
        char magic[4];

        UINT nSubMeshes;

        UINT vertexCount;
        UINT indexCount;

        UINT aabbOffset;

        UINT nameTableOffset;
        UINT nameOffsetLength;
    };

    Resource::Ref MeshLoader::LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        std::string extension = path.extension().string();
            return LoadWithAssimp(path, loadDesc);

        if (loadDesc.extension == FileExtension::OBJ) {
            return LoadObjFromFile(path, loadDesc);
        }
        else if (loadDesc.extension == FileExtension::GLTF || loadDesc.extension == FileExtension::GLB) {
            WILEY_DEBUGBREAK;
            return LoadGLTFFromFile(path, loadDesc);
            std::cout << "Not loading GLTF.GLB files." << std::endl;
        }
    }


    void OptimizeMesh(std::vector<Vertex>& vertices, std::vector<UINT>& indices)
    {

            // 1. Remove duplicate vertices
            std::vector<uint32_t> remap(vertices.size());

            try
            {
                size_t newVertexCount = meshopt_generateVertexRemap(
                    remap.data(),
                    indices.data(),
                    indices.size(),
                    vertices.data(),
                    vertices.size(),
                    sizeof(Vertex)
                );

                // Remap vertex buffer
                std::vector<Vertex> newVertices(newVertexCount);
                meshopt_remapVertexBuffer(
                    newVertices.data(),
                    vertices.data(),
                    vertices.size(),
                    sizeof(Vertex),
                    remap.data()
                );

                //Remap index buffer
                meshopt_remapIndexBuffer(
                    indices.data(),
                    indices.data(),
                    indices.size(),
                    remap.data()
                );

                vertices = std::move(newVertices);
            }
            catch (std::exception& e) {
                std::cout << "Failed to remap vertex data. No guarantee model is fully optimized.\n" << std::endl;
            }


            //Vertex Cache Optimization
            std::vector<uint32_t> cacheOptimized(indices.size());
            meshopt_optimizeVertexCache(
                cacheOptimized.data(),
                indices.data(),
                indices.size(),
                vertices.size()
            );
            indices = std::move(cacheOptimized);


            //Overdraw Optimization
            meshopt_optimizeOverdraw(
                indices.data(),
                indices.data(),
                indices.size(),
                (float*)vertices.data(),
                vertices.size(),
                sizeof(Vertex),
                1.05f
            );


            //Vertex Fetch Optimization
            meshopt_optimizeVertexFetch(
                vertices.data(),
                indices.data(),
                indices.size(),
                vertices.data(),
                vertices.size(),
                sizeof(Vertex)
            );
    }

    std::vector<float> GetLODFractions(LODDecayType type, UINT lodCount) {
        std::vector<float> lodFractions;
        if (lodCount == 0) return lodFractions;

        switch (type) {
        case LODDecayType::Exponential:
            for (UINT i = 0; i < lodCount; ++i) {
                float t = float(i) / float(std::max<UINT>(1, lodCount - 1));
                float fraction = std::pow(0.1f, t);
                lodFractions.push_back(fraction);
            }
            break;

        case LODDecayType::Linear:
            for (UINT i = 0; i < lodCount; ++i) {
                float t = float(i) / float(std::max<UINT>(1, lodCount - 1));
                float fraction = 1.0f - t * 0.9f;
                lodFractions.push_back(fraction);
            }
            break;

        case LODDecayType::HalfLife:
            for (UINT i = 0; i < lodCount; ++i) {
                float fraction = std::pow(0.5f, float(i));
                fraction = std::max(fraction, 0.005f);
                lodFractions.push_back(fraction);
            }
            break;
        }

        return lodFractions;
    }

    void MeshLoader::GenerateLevelOfDetail(LODDecayType type, UINT requestedLodCount, std::vector<Vertex>& vertices, std::vector<UINT>& indices, Mesh& meshData) {

        if (requestedLodCount < 2) return;
        if (!resourceCache) {
            std::cout << "[LOD] resourceCache is null, aborting LOD generation.\n";
            return;
        }
        if (!resourceCache->indexPool) {
            std::cout << "[LOD] indexPool is null, aborting LOD generation.\n";
            return;
        }
        if (indices.empty() || vertices.empty()) return;

        std::vector<float> lodFractions = GetLODFractions(type, requestedLodCount);

        bool skipFirstIfFull = true;
        for (size_t fi = 0; fi < lodFractions.size(); ++fi) {
            float fraction = lodFractions[fi];

            size_t targetCount = std::max<size_t>(3, size_t(std::round(fraction * double(indices.size()))));

            if (skipFirstIfFull && fi == 0 && targetCount >= indices.size()) {
                continue;
            }

            std::vector<uint32_t> lodIndices(targetCount);

            size_t actualCount = meshopt_simplify(
                lodIndices.data(),
                indices.data(),
                indices.size(),
                (float*)vertices.data(),
                vertices.size(),
                sizeof(Vertex),
                targetCount,
                0.1f
            );

            if (actualCount == 0)
                continue;

            WILEY_MUSTBE_UINTSIZE(actualCount);

            MemoryBlock<UINT> lodIndexBlock = resourceCache->indexPool->Allocate(static_cast<uint32_t>(actualCount));

            memcpy(lodIndexBlock.data(), lodIndices.data(), actualCount * sizeof(uint32_t));

            meshData.lodIndexBlocks.push_back(lodIndexBlock);
        }
    }


    Resource::Ref MeshLoader::LoadObjFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        std::shared_ptr<Mesh> meshRef = std::make_shared<Mesh>();
        Mesh& meshData = *meshRef.get();

        tinyobj::ObjReaderConfig config;
        tinyobj::ObjReader reader;
        config.triangulate = true;

        if (!reader.ParseFromFile(path.string(), config))
        {
            if (!reader.Error().empty())
            {
                std::cout << "[TinyObj]: " << reader.Error() << std::endl;
            }
            return nullptr;
        }
        if (!reader.Warning().empty())
        {
            std::cout << "[TinyObj]: " << reader.Warning() << std::endl;
        }

        const tinyobj::attrib_t& attrib = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
        const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

        meshData.subMeshes.resize(shapes.size());
        meshData.boxes.resize(shapes.size());
        meshData.names.resize(shapes.size());

        std::vector<Vertex> vertices;
        std::vector<UINT> indices;

        for (const auto& mtl : materials) {
            std::string str = mtl.diffuse_texname;
            str = mtl.bump_texname;
            str = mtl.normal_texname;
            str = mtl.alpha_texname;
            str = mtl.ambient_texname;
            str = mtl.metallic_texname;
            str = mtl.specular_texname;
            str = mtl.roughness_texname;
        }

        // Load Geometry Data
        for (size_t s = 0; s < shapes.size(); s++)
        {
            const auto& shape = shapes[s];

            meshData.subMeshes[s].nameOffset = meshData.names.size();
            meshData.names += shape.name;
            meshData.subMeshes[s].nameCharCount = shape.name.size();
            meshData.subMeshes[s].vertexOffset = vertices.size();
            meshData.subMeshes[s].indexOffset = indices.size();
            meshData.subMeshes[s].index = s;


            AABB meshBoundingBox{};

            size_t index_offset = 0;
            size_t indexCount = 0;


            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                int fv = shape.mesh.num_face_vertices[f];

                for (int v = 0; v < fv; v++)
                {
                    auto idx = shape.mesh.indices[index_offset + v];

                    if (idx.vertex_index < 0 || idx.vertex_index * 3 + 2 >= attrib.vertices.size())
                    {
                        std::cout << "[TinyObj]: Vertex index out of bounds" << std::endl;
                        continue;
                    }

                    DirectX::XMFLOAT3 pos = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };

                    meshBoundingBox.min.x = std::min(meshBoundingBox.min.x, pos.x);
                    meshBoundingBox.min.y = std::min(meshBoundingBox.min.y, pos.y);
                    meshBoundingBox.min.z = std::min(meshBoundingBox.min.z, pos.z);

                    meshBoundingBox.max.x = std::max(meshBoundingBox.max.x, pos.x);
                    meshBoundingBox.max.y = std::max(meshBoundingBox.max.y, pos.y);
                    meshBoundingBox.max.z = std::max(meshBoundingBox.max.z, pos.z);



                    meshData.aabb.min.x = std::min(meshData.aabb.min.x, pos.x);
                    meshData.aabb.min.y = std::min(meshData.aabb.min.y, pos.y);
                    meshData.aabb.min.z = std::min(meshData.aabb.min.z, pos.z);

                    meshData.aabb.max.x = std::max(meshData.aabb.max.x, pos.x);
                    meshData.aabb.max.y = std::max(meshData.aabb.max.y, pos.y);
                    meshData.aabb.max.z = std::max(meshData.aabb.max.z, pos.z);


                    DirectX::XMFLOAT2 uv = { 0.0f, 0.0f };
                    if (idx.texcoord_index != -1)
                    {
                        if (idx.texcoord_index * 2 + 1 < attrib.texcoords.size())
                        {
                            uv = {
                                attrib.texcoords[2 * idx.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // Flip V
                            };
                        }
                    }

                    DirectX::XMFLOAT3 normal{ 0.0f, 1.0f, 0.0f };
                    if (idx.normal_index != -1)
                    {
                        if (idx.normal_index * 3 + 2 < attrib.normals.size())
                        {
                            normal = {
                                attrib.normals[3 * idx.normal_index + 0],
                                attrib.normals[3 * idx.normal_index + 1],
                                attrib.normals[3 * idx.normal_index + 2]
                            };
                        }
                    }

                    DirectX::XMFLOAT4 tangent = { 1.0f, 1.0f, 1.0f,1.0f };

                    vertices.push_back({ pos,normal,uv, tangent, static_cast<UINT>(s) });

                    size_t vertexIndex = vertices.size() - 1;
                    if (vertexIndex > std::numeric_limits<UINT>::max())
                    {
                        std::cout << "[TinyObj]: Too many vertices for index type" << std::endl;
                        continue;
                    }
                    indices.push_back(static_cast<UINT>(vertexIndex));
                    indexCount++;
                }
                index_offset += fv;
            }

            meshData.subMeshes[s].vertexCount = vertices.size() - meshData.subMeshes[s].vertexOffset;
            meshData.subMeshes[s].indexCount = indices.size() - meshData.subMeshes[s].indexOffset;

            meshData.boxes[s] = meshBoundingBox;

            WILEY_MUSTBE_UINTSIZE(meshData.subMeshes[s].vertexCount);
            WILEY_MUSTBE_UINTSIZE(meshData.subMeshes[s].indexCount);

            meshData.vertexCount += meshData.subMeshes[s].vertexCount;
            meshData.indexCount += meshData.subMeshes[s].indexCount;
        }


        OptimizeMesh(vertices, indices);

       //GenerateLevelOfDetail(loadDesc.desc.mesh.decayType, loadDesc.desc.mesh.lodCount, vertices, indices, meshData);

        MemoryBlock<Vertex> vertexMemBlk = resourceCache->vertexPool->Allocate(vertices.size());
        MemoryBlock<UINT> indexMemBlk = resourceCache->indexPool->Allocate(indices.size());

        memcpy(vertexMemBlk.data(), vertices.data(), vertexMemBlk.size_bytes());
        memcpy(indexMemBlk.data(), indices.data(), indexMemBlk.size_bytes());

       /* meshData.vertexBlock = vertexMemBlk;
        meshData.indexBlock = indexMemBlk;*/

        meshData.vertexOffset = resourceCache->vertexPool->GetIndex(vertexMemBlk);
        meshData.indexOffset = resourceCache->indexPool->GetIndex(indexMemBlk);

        return meshRef;
    }

    Resource::Ref MeshLoader::LoadGLTFFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        WILEY_DEBUGBREAK;
        std::shared_ptr<Mesh> meshRef = std::make_shared<Mesh>();
        Mesh& meshData = *meshRef.get();
        return nullptr;
    }


    void MeshLoader::SaveToFile(filespace::filepath path, Mesh* meshResource)
    {
        std::ofstream file(path.string(), std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to write mesh data to file." << std::endl;
            return;
        }

        _MeshFileHead fileHeader{};
        fileHeader.magic[0] = 'W';
        fileHeader.magic[1] = 'I';
        fileHeader.magic[2] = 'L';
        fileHeader.magic[3] = 'Y';

        fileHeader.nSubMeshes = meshResource->subMeshes.size();

        fileHeader.indexCount = meshResource->indexCount;
        fileHeader.vertexCount = meshResource->vertexCount;

        MemoryBlock<Vertex> vertexBlock = MemoryBlock<Vertex>(resourceCache->vertexPool->GetPointerByIndex(meshResource->vertexOffset), meshResource->vertexCount);
        MemoryBlock<UINT> indexBlock = MemoryBlock<UINT>(resourceCache->indexPool->GetPointerByIndex(meshResource->indexOffset), meshResource->indexCount);

        fileHeader.aabbOffset = WILEY_SIZEOF(_MeshFileHead) + (WILEY_SIZEOF(SubMesh) * fileHeader.nSubMeshes)
            + vertexBlock.size_bytes() + indexBlock.size_bytes();

        fileHeader.nameTableOffset = fileHeader.aabbOffset + WILEY_SIZEOF(AABB)
            + (WILEY_SIZEOF(AABB) * fileHeader.nSubMeshes);

        file.write(reinterpret_cast<char*>(&fileHeader), WILEY_SIZEOF(_MeshFileHead));
        file.write(reinterpret_cast<char*>(meshResource->subMeshes.data()), WILEY_SIZEOF(SubMesh) * fileHeader.nSubMeshes);

        

        file.write(reinterpret_cast<char*>(vertexBlock.data()), vertexBlock.size_bytes());
        file.write(reinterpret_cast<char*>(indexBlock.data()), indexBlock.size_bytes());

        file.write(reinterpret_cast<char*>(&meshResource->aabb), WILEY_SIZEOF(AABB));
        file.write(reinterpret_cast<char*>(meshResource->boxes.data()), WILEY_SIZEOF(AABB) * fileHeader.nSubMeshes);

        std::string nam = meshResource->GetName();
        file.write(reinterpret_cast<char*>(nam.data()), nam.size());
        file.write(reinterpret_cast<char*>(meshResource->names.data()), meshResource->names.size());

        WILEY_DEBUGBREAK;
        //No file was saved to disk

        file.close();
    }

    ImageTexture* LoadImageTextureDep(ResourceCache* resourceCache, aiMaterial* material, MapType type,std::string_view modelDirectory) {
        aiReturn status;
        aiString localTexturePath;

        ResourceLoadDesc loadDesc{};
        loadDesc.desc.imageTextureDesc.type = type;

        status = material->GetTexture(aiTextureType_DIFFUSE, 0, &localTexturePath);
        const std::string cTexturePath = modelDirectory.data() + std::string("/") + localTexturePath.C_Str();
        auto imageTexture = static_cast<ImageTexture*>(resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc).get());
        return imageTexture;
    }

    void MeshLoader::ProcessNode(aiNode* node, const aiScene* scene, std::vector<Vertex>& vertices,
        std::vector<UINT>& indices, Mesh& meshData, std::filesystem::path modelDirectory)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; i++) {

            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

           {
                ResourceLoadDesc loadDesc{};

                aiReturn status;

                aiColor4D diffuseColor;
                std::string cTexturePath;
                aiString texturePath;

                loadDesc.desc.imageTextureDesc.type = MapType::Albedo;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != AI_SUCCESS)
                    material->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath);
                cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                auto albedoResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);

                loadDesc.desc.imageTextureDesc.type = MapType::Normal;
                Resource::Ref normalResource;
                if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
                {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    normalResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else  if (material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS) {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    normalResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else  if (material->GetTexture(aiTextureType_DISPLACEMENT, 0, &texturePath) == AI_SUCCESS) {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    normalResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else {
                    normalResource = resourceCache->GetDefaultImageTexture(MapType::Normal);
                }


                loadDesc.desc.imageTextureDesc.type = MapType::Roughness;
                Resource::Ref roughnessResource;
                if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) == AI_SUCCESS) {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    roughnessResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else if (material->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS) {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    roughnessResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else {
                    roughnessResource = resourceCache->GetDefaultImageTexture(MapType::Roughness);
                }

                loadDesc.desc.imageTextureDesc.type = MapType::AO;
                Resource::Ref aoResource;
                if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS) {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    aoResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &texturePath) == AI_SUCCESS) {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    aoResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else {
                    aoResource = resourceCache->GetDefaultImageTexture(MapType::AO);
                }


                loadDesc.desc.imageTextureDesc.type = MapType::Metalloic;
                status = material->GetTexture(aiTextureType_METALNESS, 0, &texturePath);
                Resource::Ref metallicResource;
                if (status == AI_SUCCESS)
                {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    metallicResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS)
                {
                    cTexturePath = modelDirectory.string() + "/" + texturePath.C_Str();
                    metallicResource = resourceCache->LoadResource<ImageTexture>(cTexturePath, loadDesc);
                }
                else {
                    metallicResource = resourceCache->GetDefaultImageTexture(MapType::Metalloic);
                }

                aiString materialName = material->GetName();
                std::string materialPath = std::string(materialName.C_Str()) + std::string(".toml");

                //CreateNew Sets Default Values.
                auto newMaterialResource = resourceCache->materialLoader->CreateNew(materialName.C_Str());
                Wiley::ResourceCache::ResourceDesc newMtlDesc = {
                    .type = ResourceType::Material,
                    .path = materialPath,
                    .state = ResourceState::NotOnDisk
                };
                resourceCache->Cache(newMaterialResource, newMtlDesc, WILEY_INVALID_UUID);
                const auto newMtlUUID = newMaterialResource->GetUUID();

                resourceCache->SetMaterialMap(newMtlUUID, albedoResource->GetUUID(), Wiley::MapType::Albedo);
                resourceCache->SetMaterialMap(newMtlUUID, normalResource->GetUUID(), Wiley::MapType::Normal);
                resourceCache->SetMaterialMap(newMtlUUID, aoResource->GetUUID(), Wiley::MapType::AO);
                resourceCache->SetMaterialMap(newMtlUUID, roughnessResource->GetUUID(), Wiley::MapType::Roughness);
                resourceCache->SetMaterialMap(newMtlUUID, metallicResource->GetUUID(), Wiley::MapType::Metalloic);

                Material* mtl = static_cast<Material*>(newMaterialResource.get());
                auto mtlData = mtl->dataPtr;

                material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
                material->Get(AI_MATKEY_METALLIC_FACTOR, mtlData->metallic.value);
                material->Get(AI_MATKEY_ROUGHNESS_FACTOR, mtlData->roughness.value);
                material->Get(AI_MATKEY_BUMPSCALING, mtlData->normal.strength);

                meshData.loadMaterials.push_back(newMtlUUID);
           } 

            SubMesh subMesh{};
            subMesh.index = meshData.subMeshes.size();

            subMesh.indexOffset = indices.size();

            subMesh.vertexCount = mesh->mNumVertices;
            subMesh.vertexOffset = vertices.size();

            subMesh.nameCharCount;
            subMesh.nameOffset;

            AABB box;
            box.min = { FLT_MAX, FLT_MAX, FLT_MAX };
            box.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

            for (int v = 0; v < mesh->mNumVertices; v++) {
                Vertex vertex = {};
                vertex.subMeshIndex = subMesh.index;

                {
                    vertex.position.x = mesh->mVertices[v].x;
                    vertex.position.y = mesh->mVertices[v].y;
                    vertex.position.z = mesh->mVertices[v].z;
                }

                box.min.x = std::min(box.min.x, vertex.position.x);
                box.min.y = std::min(box.min.y, vertex.position.y);
                box.min.z = std::min(box.min.z, vertex.position.z);
                box.max.x = std::max(box.max.x, vertex.position.x);
                box.max.y = std::max(box.max.y, vertex.position.y);
                box.max.z = std::max(box.max.z, vertex.position.z);

                meshData.aabb.min.x = std::min(meshData.aabb.min.x, vertex.position.x);
                meshData.aabb.min.y = std::min(meshData.aabb.min.y, vertex.position.y);
                meshData.aabb.min.z = std::min(meshData.aabb.min.z, vertex.position.z);
                meshData.aabb.max.x = std::max(meshData.aabb.max.x, vertex.position.x);
                meshData.aabb.max.y = std::max(meshData.aabb.max.y, vertex.position.y);
                meshData.aabb.max.z = std::max(meshData.aabb.max.z, vertex.position.z);

                if (mesh->HasNormals())
                {
                    vertex.normal.x = mesh->mNormals[v].x;
                    vertex.normal.y = mesh->mNormals[v].y;
                    vertex.normal.z = mesh->mNormals[v].z;
                }

                if (mesh->mTextureCoords[0])
                {
                    vertex.uv.x = mesh->mTextureCoords[0][v].x;
                    vertex.uv.y = mesh->mTextureCoords[0][v].y;
                }

                if (mesh->HasTangentsAndBitangents())
                {
                    DirectX::XMVECTOR N = DirectX::XMVectorSet(
                        mesh->mNormals[v].x,
                        mesh->mNormals[v].y,
                        mesh->mNormals[v].z,
                        0.0f
                    );

                    DirectX::XMVECTOR T = DirectX::XMVectorSet(
                        mesh->mTangents[v].x,
                        mesh->mTangents[v].y,
                        mesh->mTangents[v].z,
                        0.0f
                    );

                    // Load Assimp's bitangent (used ONLY to determine handedness)
                    DirectX::XMVECTOR assimpB = DirectX::XMVectorSet(
                        mesh->mBitangents[v].x,
                        mesh->mBitangents[v].y,
                        mesh->mBitangents[v].z,
                        0.0f
                    );

                    // Calculate what bitangent SHOULD be: B = N � T
                    DirectX::XMVECTOR calculatedB = DirectX::XMVector3Cross(N, T);

                    // Determine handedness by comparing directions
                    // If calculatedB and assimpB point in same direction: handedness = +1
                    // If they point in opposite directions: handedness = -1
                    float dotProduct = DirectX::XMVectorGetX(DirectX::XMVector3Dot(calculatedB, assimpB));
                    float handedness = (dotProduct < 0.0f) ? -1.0f : 1.0f;

                    // Store tangent.xyz and handedness in tangent.w
                    vertex.tangent.x = mesh->mTangents[v].x;
                    vertex.tangent.y = mesh->mTangents[v].y;
                    vertex.tangent.z = mesh->mTangents[v].z;
                    vertex.tangent.w = handedness;


                }

                vertices.push_back(vertex);
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; f++)
            {
                aiFace face = mesh->mFaces[f];
                for (unsigned int idx = 0; idx < face.mNumIndices; idx++)
                {
                    indices.push_back(static_cast<UINT>(subMesh.vertexOffset) + face.mIndices[idx]);
                }
            }

            subMesh.indexCount = indices.size() - subMesh.indexOffset;
            meshData.subMeshes.push_back(subMesh);
            meshData.boxes.push_back(box);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene, vertices, indices, meshData, modelDirectory);
        }
    }

    Resource::Ref MeshLoader::LoadWithAssimp(filespace::filepath path, ResourceLoadDesc& loadDesc) {
        std::shared_ptr<Mesh> meshRef = std::make_shared<Mesh>();
        Mesh& meshData = *meshRef.get();

        Assimp::Importer importer;

        unsigned int flags = aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices;
        flags |= (loadDesc.desc.meshDesc.normalType == NormalType::Smooth) ? aiProcess_GenSmoothNormals : aiProcess_GenNormals;

        const aiScene* scene = importer.ReadFile(path.string().c_str(), flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            return nullptr;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        ProcessNode(scene->mRootNode, scene, vertices, indices, meshData, path.parent_path());

        OptimizeMesh(vertices, indices);

        MemoryBlock<Vertex> vertexMemBlk = resourceCache->vertexPool->Allocate(vertices.size());
        MemoryBlock<UINT> indexMemBlk = resourceCache->indexPool->Allocate(indices.size());

        memcpy(vertexMemBlk.data(), vertices.data(), vertexMemBlk.size_bytes());
        memcpy(indexMemBlk.data(), indices.data(), indexMemBlk.size_bytes());

        meshData.vertexOffset = resourceCache->vertexPool->GetIndex(vertexMemBlk);
        meshData.indexOffset = resourceCache->indexPool->GetIndex(indexMemBlk);

        meshData.vertexCount = vertices.size();
        meshData.indexCount = indices.size();

        return meshRef;
    }

}