//=================================================================================================
//
//  SHforHLSL - Spherical harmonics suppport library for HLSL 2021, by MJP
//  https://github.com/TheRealMJP/SHforHLSL
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//================================================================================================

#include "SH_Lite.hlsli"

void TestOperatorOverloads()
{
    {
        SH::L1 sh = SH::L1::Zero();
        sh = SH::Add(sh, SH::L1::Zero());
        sh = SH::Subtract(sh, SH::L1::Zero());
        sh = SH::Multiply(sh, 1.0f);
        sh = SH::Divide(sh, 1.0f);
    }

    {
        SH::L1_RGB sh = SH::L1_RGB::Zero();
        sh = SH::Add(sh, SH::L1_RGB::Zero());
        sh = SH::Subtract(sh, SH::L1_RGB::Zero());
        sh = SH::Multiply(sh, 1.0f);
        sh = SH::Divide(sh, 1.0f);
    }

    {
        SH::L2 sh = SH::L2::Zero();
        sh = SH::Add(sh, SH::L2::Zero());
        sh = SH::Subtract(sh, SH::L2::Zero());
        sh = SH::Multiply(sh, 1.0f);
        sh = SH::Divide(sh, 1.0f);
    }

    {
        SH::L2_RGB sh = SH::L2_RGB::Zero();
        sh = SH::Add(sh, SH::L2_RGB::Zero());
        sh = SH::Subtract(sh, SH::L2_RGB::Zero());
        sh = SH::Multiply(sh, 1.0f);
        sh = SH::Divide(sh, 1.0f);
    }
}

void TestBasics()
{
    {
        SH::L1 a = SH::L1::Zero();
        SH::L1 b = SH::L1::Zero();
        a = SH::Lerp(a, b, 0.5f);
        float v = SH::DotProduct(a, b);
        v = SH::Evaluate(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::ConvolveWithZH(b, float2(1.0f, 1.0f));
        a = SH::ConvolveWithCosineLobe(a);
        a = SH::ConvolveWithGGX(b, 0.5f);
        v = SH::CalculateIrradiance(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::Rotate(a, float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
        SH::L1_RGB rgb = SH::ToRGB(SH::L1::Zero());
    }

    {
        SH::L1_RGB a = SH::L1_RGB::Zero();
        SH::L1_RGB b = SH::L1_RGB::Zero();
        a = SH::Lerp(a, b, 0.5f);
        float3 v = SH::DotProduct(a, b);
        v = SH::Evaluate(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::ConvolveWithZH(b, float2(1.0f, 1.0f));
        a = SH::ConvolveWithCosineLobe(a);
        a = SH::ConvolveWithGGX(b, 0.5f);
        v = SH::CalculateIrradiance(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::Rotate(a, float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
    }

    {
        SH::L2 a = SH::L2::Zero();
        SH::L2 b = SH::L2::Zero();
        a = SH::Lerp(a, b, 0.5f);
        float v = SH::DotProduct(a, b);
        v = SH::Evaluate(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::ConvolveWithZH(b, float3(1.0f, 1.0f, 1.0f));
        a = SH::ConvolveWithCosineLobe(a);
        a = SH::ConvolveWithGGX(b, 0.5f);
        v = SH::CalculateIrradiance(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::Rotate(a, float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
        SH::L2_RGB rgb = SH::ToRGB(SH::L2::Zero());
    }

    {
        SH::L2_RGB a = SH::L2_RGB::Zero();
        SH::L2_RGB b = SH::L2_RGB::Zero();
        a = SH::Lerp(a, b, 0.5f);
        float3 v = SH::DotProduct(a, b);
        v = SH::Evaluate(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::ConvolveWithZH(b, float3(1.0f, 1.0f, 1.0f));
        a = SH::ConvolveWithCosineLobe(a);
        a = SH::ConvolveWithGGX(b, 0.5f);
        v = SH::CalculateIrradiance(a, float3(0.0f, 1.0f, 0.0f));
        a = SH::Rotate(a, float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
    }
}

void TestL1Specifics()
{
    {
        SH::L1 sh = SH::ProjectOntoL1(float3(0.0f, 1.0f, 0.0f), 1.0f);
        float3 d = SH::OptimalLinearDirection(sh);
        float v = 0.0f;
        SH::ApproximateDirectionalLight(sh, d, v);
        v = SH::CalculateIrradianceGeomerics(sh, float3(0.0f, 1.0f, 0.0f));
        v = SH::CalculateIrradianceL1ZH3Hallucinate(sh, float3(0.0f, 1.0f, 0.0f));
        float2 zh = SH::ApproximateGGXAsL1ZH(0.5f);
        float s = 0.0f;
        SH::ExtractSpecularDirLight(sh, 0.5f, d, v, s);
    }

    {
        SH::L1_RGB sh = SH::ProjectOntoL1_RGB(float3(0.0f, 1.0f, 0.0f), float3(1.0f, 1.0f, 1.0f));
        float3 d = SH::OptimalLinearDirection(sh);
        float3 v = 0.0f;
        SH::ApproximateDirectionalLight(sh, d, v);
        v = SH::CalculateIrradianceGeomerics(sh, float3(0.0f, 1.0f, 0.0f));
        v = SH::CalculateIrradianceL1ZH3Hallucinate(sh, float3(0.0f, 1.0f, 0.0f));
        float s = 0.0f;
        SH::ExtractSpecularDirLight(sh, 0.5f, d, v, s);
    }
}

void TestL2Specifics()
{
    {
        SH::L2 sh = SH::ProjectOntoL2(float3(0.0f, 1.0f, 0.0f), 1.0f);
        SH::L1 l1 = SH::L2toL1(sh);
        float3 zh = SH::ApproximateGGXAsL2ZH(0.5f);
    }

    {
        SH::L2_RGB sh = SH::ProjectOntoL2_RGB(float3(0.0f, 1.0f, 0.0f), 1.0f);
        SH::L1_RGB l1 = SH::L2toL1(sh);
    }
}

[numthreads(1, 1, 1)]
void CompileTest()
{
    TestOperatorOverloads();

    TestBasics();

    TestL1Specifics();

    TestL2Specifics();
}