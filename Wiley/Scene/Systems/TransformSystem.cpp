#include "TransformSystem.h"
#include "../Entity.h"
#include "../Component.h"



namespace Wiley
{
	void TransformSystem::OnUpdate(float dt)
	{
        for (auto& entt : scene->GetComponentView<TransformComponent>())
        {
            Entity entity(entt, scene);
            auto& transform = entity.GetComponent<TransformComponent>();

            float rx = DirectX::XMConvertToRadians(transform.rotation.x);
            float ry = DirectX::XMConvertToRadians(transform.rotation.y);
            float rz = DirectX::XMConvertToRadians(transform.rotation.z);

            DirectX::XMMATRIX scaleMatrix =
                DirectX::XMMatrixScaling(
                    transform.scale.x,
                    transform.scale.y,
                    transform.scale.z
                );

            DirectX::XMMATRIX rotationMatrix =
                DirectX::XMMatrixRotationX(rx) *
                DirectX::XMMatrixRotationY(ry) *
                DirectX::XMMatrixRotationZ(rz);

            DirectX::XMMATRIX translationMatrix =
                DirectX::XMMatrixTranslation(
                    transform.position.x,
                    transform.position.y,
                    transform.position.z
                );
             
            DirectX::XMMATRIX model =
                translationMatrix *   
                rotationMatrix *      
                scaleMatrix;          
            
            DirectX::XMStoreFloat4x4(&transform.modelMatrix, DirectX::XMMatrixTranspose(model));
        }
	}
}
