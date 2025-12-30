#include "LightComponentSystem.h"
#include "../Entity.h"

//Todo: No need to run this every frame.

namespace Wiley {

    void LightComponentSystem::Execute(void* data) {
        LightComponent* light = (LightComponent*)data;
        switch (light->type) {
            case LightType::Directional:
            {
                ComputeDirectionalLightViewProjections(light);
                break;
            }
            case LightType::Point:
            {
                ComputePointLightViewProjections(light);
                break;
            }
            case LightType::Spot:
            {
                ComputeSpotLightViewProjection(light);
                break;
            }
        }
    }

	void LightComponentSystem::OnUpdate(float dt)
	{
        const auto smm = scene->GetShadowMapManager();
        auto dirtyComps = smm->GetDirtyEntities();

        for (int i = 0; i < dirtyComps.size(); i++) {
            Entity entity = Entity(dirtyComps.front(), scene);;
            auto& light = entity.GetComponent<LightComponent>();

            Execute(&light);
            dirtyComps.pop();
        }

        
        if (!smm->IsAllLightEntitiesDirty())
            return;

        //Update all.
		for (auto& entt : scene->GetComponentView<LightComponent>())
		{
			Entity entity(entt, scene);
			auto& light = entity.GetComponent<LightComponent>();

            Execute(&light);
		}

	}

	void LightComponentSystem::ComputePointLightViewProjections(void* lightComponent)
	{
		using namespace DirectX;

		LightComponent* light = (LightComponent*)lightComponent;

		std::array<XMMATRIX, 6> shadowVPs;

		XMVECTOR lightPos = XMLoadFloat3(&light->position);
		const float nearPlane = 0.1f;
		const float farPlane = light->intensity;

		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, nearPlane, farPlane);

		XMVECTOR targets[6] = {
			lightPos + XMVectorSet( 1, 0, 0, 0), // +X
			lightPos + XMVectorSet(-1, 0, 0, 0), // -X
			lightPos + XMVectorSet( 0, 1, 0, 0), // +Y
			lightPos + XMVectorSet( 0,-1, 0, 0), // -Y
			lightPos + XMVectorSet( 0, 0, 1, 0), // +Z
			lightPos + XMVectorSet( 0, 0,-1, 0)  // -Z
		};

		const static XMVECTOR up[6] = {
			XMVectorSet(0, 1, 0, 0),  // +X
			XMVectorSet(0, 1, 0, 0),  // -X
			XMVectorSet(0, 0,-1, 0),  // +Y 
			XMVectorSet(0, 0, 1, 0),  // -Y 
			XMVectorSet(0, 1, 0, 0),  // +Z
			XMVectorSet(0, 1, 0, 0)   // -Z
		};

		for (int i = 0; i < 6; i++) {
			XMMATRIX view = XMMatrixLookAtLH(lightPos, targets[i], up[i]);
			shadowVPs[i] = view * proj;
		}

		DirectX::XMFLOAT4X4* matrixDataHead = scene->GetShadowMapManager()->GetLightProjection(light->matrixIndex);

		int i = 0;
		for (const auto& vp : shadowVPs) {
            DirectX::XMStoreFloat4x4(matrixDataHead + i, DirectX::XMMatrixTranspose(vp));
			i++;
		}
	}


    void LightComponentSystem::ComputeDirectionalLightViewProjections(void* lightComponent)
    {
        using namespace DirectX;
        LightComponent* light = (LightComponent*)lightComponent;

        const auto camera = scene->GetCamera();
        const auto cameraNear = camera->GetNear();
        const auto cameraFar = camera->GetFar();

        const int numCascades = 4;
        const float cascadeSplits[5] = { cameraNear, 10.0f, 30.0f, 60.0f, cameraFar }; // Adjust splits as needed

        WILEY_DEBUGBREAK; //Fix cascade splits...

        XMVECTOR lightDir = XMLoadFloat3(&light->position);
        lightDir = XMVector3Normalize(lightDir);

        XMMATRIX invCameraViewProj = camera->GetInverseViewProjection();

        std::array<XMMATRIX, 4> shadowVPs;

        for (int cascade = 0; cascade < numCascades; cascade++)
        {
            float nearPlane = cascadeSplits[cascade];
            float farPlane = cascadeSplits[cascade + 1];

            XMVECTOR frustumCorners[8] = {
                XMVectorSet(-1.0f, -1.0f, 0.0f, 1.0f), // near
                XMVectorSet(1.0f, -1.0f, 0.0f, 1.0f),
                XMVectorSet(-1.0f,  1.0f, 0.0f, 1.0f),
                XMVectorSet(1.0f,  1.0f, 0.0f, 1.0f),
                XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f), // far
                XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f),
                XMVectorSet(-1.0f,  1.0f, 1.0f, 1.0f),
                XMVectorSet(1.0f,  1.0f, 1.0f, 1.0f)
            };

            for (int i = 0; i < 8; i++)
            {
                XMVECTOR corner = XMVector4Transform(frustumCorners[i], invCameraViewProj);
                frustumCorners[i] = corner / XMVectorGetW(corner);
            }

            float nearRatio = (nearPlane - cameraNear) / (cameraFar - cameraNear);
            float farRatio = (farPlane - cameraNear) / (cameraFar - cameraNear);

            for (int i = 0; i < 4; i++)
            {
                XMVECTOR nearToFar = frustumCorners[i + 4] - frustumCorners[i];
                frustumCorners[i] = frustumCorners[i] + nearToFar * nearRatio;
                frustumCorners[i + 4] = frustumCorners[i] + nearToFar * (farRatio - nearRatio);
            }

            XMVECTOR center = XMVectorZero();
            for (int i = 0; i < 8; i++)
                center += frustumCorners[i];
            center /= 8.0f;

            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            if (abs(XMVectorGetY(lightDir)) > 0.99f)
                up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

            XMMATRIX lightView = XMMatrixLookAtLH(center - lightDir * 50.0f, center, up);

            float minX = FLT_MAX, maxX = -FLT_MAX;
            float minY = FLT_MAX, maxY = -FLT_MAX;
            float minZ = FLT_MAX, maxZ = -FLT_MAX;

            for (int i = 0; i < 8; i++)
            {
                XMVECTOR lightSpaceCorner = XMVector3Transform(frustumCorners[i], lightView);
                minX = std::min(minX, XMVectorGetX(lightSpaceCorner));
                maxX = std::max(maxX, XMVectorGetX(lightSpaceCorner));
                minY = std::min(minY, XMVectorGetY(lightSpaceCorner));
                maxY = std::max(maxY, XMVectorGetY(lightSpaceCorner));
                minZ = std::min(minZ, XMVectorGetZ(lightSpaceCorner));
                maxZ = std::max(maxZ, XMVectorGetZ(lightSpaceCorner));
            }

            minZ -= 50.0f;

            XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
            shadowVPs[cascade] = lightView * lightProj;
        }

        DirectX::XMFLOAT4X4* matrixDataHead = scene->GetShadowMapManager()->GetLightProjection(light->matrixIndex);
        for (int i = 0; i < numCascades; i++)
        {
            DirectX::XMStoreFloat4x4(matrixDataHead + i, DirectX::XMMatrixTranspose(shadowVPs[i]));
        }
    }

    void LightComponentSystem::ComputeSpotLightViewProjection(void* lightComponent)
    {
        using namespace DirectX;
        LightComponent* light = (LightComponent*)lightComponent;

        XMVECTOR lightPos = XMLoadFloat3(&light->position);
        XMVECTOR lightDir = XMLoadFloat3(&light->spotDirection);
        lightDir = XMVector3Normalize(lightDir);

        const float nearPlane = 0.1f;
        const float farPlane = light->intensity; 
        const float spotAngle = light->outerRadius; 

        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        if (abs(XMVectorGetY(lightDir)) > 0.99f)
            up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        XMVECTOR target = lightPos + lightDir;
        XMMATRIX view = XMMatrixLookAtLH(lightPos, target, up);

        float aspectRatio = 1.0f;
        XMMATRIX proj = XMMatrixPerspectiveFovLH(spotAngle, aspectRatio, nearPlane, farPlane);

        XMMATRIX shadowVP = view * proj;

        DirectX::XMFLOAT4X4* matrixDataHead = scene->GetShadowMapManager()->GetLightProjection(light->matrixIndex);
        DirectX::XMStoreFloat4x4(matrixDataHead, DirectX::XMMatrixTranspose(shadowVP));
    }
}