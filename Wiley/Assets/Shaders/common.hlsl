struct Plane
{
    float3 normal;
    float distance;
};

struct Frustum
{
    //Order => left,right,top,bottom
    Plane planes[4];
};

struct Sphere
{
    float3 center;
    float radius;
};

struct Cluster
{
    float3 min;
    float3 max;
};

struct ClusterData
{
    uint size;
    uint offset;
};

struct ComputeInput
{
    uint3 dispatchID : SV_DispatchThreadID;
    uint3 groupThreadID : SV_GroupThreadID;
    uint3 groupID : SV_GroupID;
};


Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;
    
    float3 v0 = p0 - p2;
    float3 v1 = p1 - p2;
    
    plane.normal = normalize(cross(v0, v1));
    
    plane.distance = dot(plane.normal, p2);
    return plane;
}

float4 ScreenToViewSpace(float4x4 inverseProjectionMatrix, float4 screen, float2 screenDimensions)
{
    float2 texCoord = screen.xy / screenDimensions.xy;

    //Convert to clipSpace
    float4 clip = float4(float2(texCoord.x, 1.0 - texCoord.y) * 2.0 - 1.0, screen.z, screen.w);

    //View space transform
    float4 view = mul(inverseProjectionMatrix, clip);

    //Perspective projection
    view = view / view.w;

    return view;
}

float3 IntersectLineToZPlane(float3 a, float3 b, float z)
{
    const float3 normal = float3(0.0f, 0.0f, 1.0f);

    float3 ab = b - a;

    float t = (z - dot(normal, a)) / dot(normal, ab);

    return a + t * ab;
}

float LinearizeDepth(float depth, float nearPlane, float farPlane)
{
    float ndc = depth * 2.0f - 1.0f;
    
    float _linear = (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - ndc * (farPlane - nearPlane));
    
    return _linear;
}

bool IntersectSphereAABB(Sphere sphere, float3 min, float3 max)
{
    float3 closest_point = clamp(sphere.center, min, max);
    
    float3 diff = sphere.center - closest_point;
    float distance_squared = dot(diff, diff);
    
    return distance_squared <= (sphere.radius * sphere.radius);
}

float3x3 Inverse3x3(float3x3 m)
{
    float det = determinant(m);
    
    if (abs(det) < 1e-6)
        return float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1); 
    
    float3x3 adj;
    
    adj[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    adj[0][1] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
    adj[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];
    
    adj[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    adj[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    adj[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];
    
    adj[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    adj[2][1] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
    adj[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    
    return adj / det;
}