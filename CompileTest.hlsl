//=================================================================================================
//
//  SHforHLSL - Spherical harmonics suppport library for HLSL 2021, by MJP
//  https://github.com/TheRealMJP/SHforHLSL
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//================================================================================================

#include "SH.hlsli"

template<typename T, int N> void TestOperatorOverloads()
{
    {
        SH::L1<T, N> sh = SH::L1<T, N>::Zero();
        sh = sh + SH::L1<T, N>::Zero();
        sh = sh - SH::L1<T, N>::Zero();
        sh = sh * T(1.0);
        sh = sh * (vector<T, N>)(1.0);
        sh = sh / T(1.0);
        sh = sh / (vector<T, N>)(1.0);
    }

    {
        SH::L2<T, N> sh = SH::L2<T, N>::Zero();
        sh = sh + SH::L2<T, N>::Zero();
        sh = sh - SH::L2<T, N>::Zero();
        sh = sh * T(1.0);
        sh = sh * (vector<T, N>)(1.0);
        sh = sh / T(1.0);
        sh = sh / (vector<T, N>)(1.0);
    }
}

template<typename T, int N> void TestBasics()
{
    {
        SH::L1<T, N> a = SH::L1<T, N>::Zero();
        SH::L1<T, N> b = SH::L1<T, N>::Zero();
        a = SH::Lerp(a, b, T(0.5));
        vector<T, N> v = SH::DotProduct(a, b);
        v = SH::Evaluate(a, vector<T, 3>(0.0, 1.0, 0.0));
        a = SH::ConvolveWithZH(b, vector<T, 2>(1.0, 1.0));
        a = SH::ConvolveWithCosineLobe(a);
        v = SH::CalculateIrradiance(a, vector<T, 3>(0.0, 1.0, 0.0));
        a = SH::Rotate(a, float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
    }

    {
        SH::L2<T, N> a = SH::L2<T, N>::Zero();
        SH::L2<T, N> b = SH::L2<T, N>::Zero();
        a = SH::Lerp(a, b, T(0.5));
        vector<T, N> v = SH::DotProduct(a, b);
        v = SH::Evaluate(a, vector<T, 3>(0.0, 1.0, 0.0));
        a = SH::ConvolveWithZH(b, vector<T, 3>(1.0, 1.0, 1.0));
        a = SH::ConvolveWithCosineLobe(a);
        v = SH::CalculateIrradiance(a, vector<T, 3>(0.0, 1.0, 0.0));
        a = SH::Rotate(a, float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
    }
}

template<typename T, int N> void TestL1Specifics()
{
    SH::L1<T, N> sh = SH::ProjectOntoL1(vector<T, 3>(0.0, 1.0, 0.0), (vector<T, N>)(1.0));
    vector<T, 3> d = SH::OptimalLinearDirection(sh);
    vector<T, N> v = (vector<T, N>)(0.0);
    SH::ApproximateDirectionalLight(sh, d, v);
    v = SH::CalculateIrradianceGeomerics(sh, vector<T, 3>(0.0, 1.0, 0.0));
    v = SH::CalculateIrradianceL1ZH3Hallucinate(sh, vector<T, 3>(0.0, 1.0, 0.0));
}

template<typename T, int N> void TestL2Specifics()
{
    SH::L2<T, N> sh = SH::L2<T, N>::Zero();
    SH::L1<T, N> l1 = SH::L2toL1(sh);
}

[numthreads(1, 1, 1)]
void CompileTest()
{
    TestOperatorOverloads<float, 1>();
    TestOperatorOverloads<float, 3>();
    TestOperatorOverloads<half, 1>();
    TestOperatorOverloads<half, 3>();

    TestBasics<float, 1>();
    TestBasics<float, 3>();
    TestBasics<half, 1>();
    TestBasics<half, 3>();

    TestL1Specifics<float, 1>();
    TestL1Specifics<float, 3>();
    TestL1Specifics<half, 1>();
    TestL1Specifics<half, 3>();

    TestL2Specifics<float, 1>();
    TestL2Specifics<float, 3>();
    TestL2Specifics<half, 1>();
    TestL2Specifics<half, 3>();
}