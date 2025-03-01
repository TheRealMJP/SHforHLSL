//=================================================================================================
//
//  SHTest
//  by MJP
//  https://therealmjp.github.io/
//
//  All code and content licensed under the MIT license
//
//=================================================================================================

#include <ShaderDebug.hlsl>
#include "AppSettings.hlsl"
#include "SharedTypes.h"

#if UseLite_
#include "../../SH_Lite.hlsli"
#else
#include "../../SH.hlsli"
#endif

ConstantBuffer<TestConstants> CB : register(b0);

struct Sphere
{
    float3 Pos;
    float Radius;
};

struct Ray
{
    float3 Origin;
    float3 Direction;
};

float RaySphereIntersection(Ray ray, Sphere sphere)
{
    float3 oc = ray.Origin - sphere.Pos;
    float b = dot(oc, ray.Direction);
    float c = dot(oc, oc) - (sphere.Radius * sphere.Radius);
    float t = (b * b) - c;
    if (t > 0.0f)
        t = -b - sqrt(t);
    return t;
}

float3 SphereNormal(float3 hitPos, Sphere sphere)
{
    return normalize(hitPos - sphere.Pos);
}

float3x3 MakeRotationY()
{
    float sinT = sin(CB.Time);
    float cosT = cos(CB.Time);
    float3 rotX = float3(cosT, 0.0f, sinT);
    float3 rotY = float3(0.0f, 1.0f, 0.0f);
    float3 rotZ = normalize(cross(rotX, rotY));
    return float3x3(rotX, rotY, rotZ);
}

#if UseLite_

float3 DoTest(float3 normal)
{
    const float3 lightDir = normalize(float3(1, 1, 0));
    const float monoLightColor = 0.5f;
    const float3 lightColor = float3(0.25f, 0.5f, 0.75f);
    const float3x3 rotation = MakeRotationY();

    if(CB.TestMode == TestMode_L1)
    {
        SH::L1 sh = SH::ProjectOntoL1(lightDir, monoLightColor);
        sh = SH::Rotate(sh, rotation);
        if (normal.y > 0.0f)
            return SH::CalculateIrradianceL1ZH3Hallucinate(sh, normal);
        else
            return SH::CalculateIrradianceGeomerics(sh, normal);
    }
    else if(CB.TestMode == TestMode_L1_RGB)
    {
        SH::L1_RGB sh = SH::ProjectOntoL1_RGB(lightDir, lightColor);
        sh = SH::Rotate(sh, rotation);
        if (normal.y > 0.0f)
            return SH::CalculateIrradianceL1ZH3Hallucinate(sh, normal);
        else
            return SH::CalculateIrradianceGeomerics(sh, normal);
    }
    else if(CB.TestMode == TestMode_L2)
    {
        SH::L2 sh = SH::ProjectOntoL2(lightDir, monoLightColor);
        sh = SH::Rotate(sh, rotation);
        return SH::CalculateIrradiance(sh, normal);
    }
    else if(CB.TestMode == TestMode_L2_RGB)
    {
        SH::L2_RGB sh = SH::ProjectOntoL2_RGB(lightDir, lightColor);
        sh = SH::Rotate(sh, rotation);
        return SH::CalculateIrradiance(sh, normal);
    }

    return 0.0f;
}

#else

float3 DoTest(float3 normal)
{
    const float3 lightDir = normalize(float3(1, 1, 0));
    const float monoLightColor = 0.5f;
    const float3 lightColor = float3(0.25f, 0.5f, 0.75f);
    const float3x3 rotation = MakeRotationY();

    if(CB.TestMode == TestMode_L1)
    {
        SH::L1 sh = SH::ProjectOntoL1(lightDir, monoLightColor);
        sh = SH::Rotate(sh, rotation);
        if (normal.y > 0.0f)
            return SH::CalculateIrradianceL1ZH3Hallucinate(sh, normal);
        else
            return SH::CalculateIrradianceGeomerics(sh, normal);
    }
    else if(CB.TestMode == TestMode_L1_RGB)
    {
        SH::L1_RGB sh = SH::ProjectOntoL1(lightDir, lightColor);
        sh = SH::Rotate(sh, rotation);
        if (normal.y > 0.0f)
            return SH::CalculateIrradianceL1ZH3Hallucinate(sh, normal);
        else
            return SH::CalculateIrradianceGeomerics(sh, normal);
    }
    else if(CB.TestMode == TestMode_L2)
    {
        SH::L2 sh = SH::ProjectOntoL2(lightDir, monoLightColor);
        sh = SH::Rotate(sh, rotation);
        return SH::CalculateIrradiance(sh, normal);
    }
    else if(CB.TestMode == TestMode_L2_RGB)
    {
        SH::L2_RGB sh = SH::ProjectOntoL2(lightDir, lightColor);
        sh = SH::Rotate(sh, rotation);
        return SH::CalculateIrradiance(sh, normal);
    }
    else if(CB.TestMode == TestMode_L1_FP16)
    {
        SH::L1_F16 sh = SH::ProjectOntoL1(half3(lightDir), half(monoLightColor));
        sh = SH::Rotate(sh, rotation);
        if (normal.y > 0.0f)
            return SH::CalculateIrradianceL1ZH3Hallucinate(sh, half3(normal));
        else
            return SH::CalculateIrradianceGeomerics(sh, half3(normal));
    }
    else if(CB.TestMode == TestMode_L1_RGB_FP16)
    {
        SH::L1_F16_RGB sh = SH::ProjectOntoL1(half3(lightDir), half3(lightColor));
        sh = SH::Rotate(sh, rotation);
        if (normal.y > 0.0f)
            return SH::CalculateIrradianceL1ZH3Hallucinate(sh, half3(normal));
        else
            return SH::CalculateIrradianceGeomerics(sh, half3(normal));
    }
    else if(CB.TestMode == TestMode_L2_FP16)
    {
        SH::L2_F16 sh = SH::ProjectOntoL2(half3(lightDir), half(monoLightColor));
        sh = SH::Rotate(sh, rotation);
        return SH::CalculateIrradiance(sh, half3(normal));
    }
    else if(CB.TestMode == TestMode_L2_RGB_FP16)
    {
        SH::L2_F16_RGB sh = SH::ProjectOntoL2(half3(lightDir), half3(lightColor));
        sh = SH::Rotate(sh, rotation);
        return SH::CalculateIrradiance(sh, half3(normal));
    }

    return 0.0f;
}

#endif

float4 SHTestPS(in float4 screenPos : SV_Position, in float2 uv : TEXCOORD) : SV_Target0
{
    float4 normalizedPos = float4(uv * 2.0f - 1.0f, 1.0f, 1.0f);
    normalizedPos.y *= -1.0f;
    float4 rayEndPos = mul(normalizedPos, CB.InvProjection);
    rayEndPos.xyz /= rayEndPos.z;

    Ray ray;
    ray.Origin = 0.0f;
    ray.Direction = normalize(rayEndPos.xyz - ray.Origin);

    Sphere sphere;
    sphere.Pos = float3(0.0f, 0.0f, 5.0f);;
    sphere.Radius = 1.0f;

    float hitDistance = RaySphereIntersection(ray, sphere);
    if(hitDistance < 0)
        discard;

    float3 hitPos = ray.Origin + hitDistance * ray.Direction;
    float3 hitNormal = SphereNormal(hitPos, sphere);

    return float4(DoTest(hitNormal), 1.0f);
}