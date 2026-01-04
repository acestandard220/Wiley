#include "MeshFilterSystem.h"
#include "../Entity.h"

namespace Wiley
{
	void MeshFilterSystem::OnUpdate(float dt)
	{
		SubMeshData* subMeshDataHead = scene->GetSubMeshDataUploadBuffer()->GetBasePointer();
		for (auto& entt : scene->GetComponentView<MeshFilterComponent>())
		{
			Entity entity(entt, scene);
			const MeshFilterComponent& meshFilter = entity.GetComponent<MeshFilterComponent>();
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();

			SubMeshData* meshFilterSubMeshDataStart = subMeshDataHead + meshFilter.subMeshDataOffset;

			std::span<SubMeshData> subMeshDataSpan(meshFilterSubMeshDataStart, meshFilter.subMeshCount);
			std::for_each(subMeshDataSpan.begin(), subMeshDataSpan.end(), [&](SubMeshData& subMeshData) {
				subMeshData.modelMatrix = transform.modelMatrix;
			});
		}

	}
}
