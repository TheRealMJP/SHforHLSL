//=================================================================================================
//
//  SH.hlsli - Spherical harmonics suppport library for HLSL 2021, by MJP
//  https://github.com/TheRealMJP/SHforHLSL
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

//=================================================================================================
//
// This header is intended to be included directly from HLSL 2021+ code, or similar. It
// implements types and utility functions for working with low-order spherical harmonics,
// focused on use cases for graphics.
//
// Currently this library has support for L1 (2 bands, 4 coefficients) and
// L2 (3 bands, 9 coefficients) SH. Depending on the author and material you're reading, you may
// see L1 referred to as both first-order or second-order, and L2 referred to as second-order
// or third-order. Ravi Ramamoorthi tends to refer to three bands as second-order, and
// Peter-Pike Sloan tends to refer to three bands as third-order. This library always uses L1 and
// L2 for clarity.
//
// The L1 and L2 core types can be templated on floating-point scalar and vector types, and are
// intended to be used with float, float3, half, and half3. Currently the direction vectors
// are passed as fp32, which results in conversions.
//
// Example #1: integrating and projecting radiance onto L2 SH
//
// SH::L2 radianceSH = SH::L2::Zero();
// for(uint sampleIndex = 0; sampleIndex < NumSamples; ++sampleIndex)
// {
//     float2 u1u2 = RandomFloat2(sampleIndex, NumSamples);
//     float3 sampleDirection = SampleDirectionSphere(u1u2);
//     float3 sampleRadiance = CalculateIncomingRadiance(sampleDirection);
//     radianceSH += SH::ProjectOntoL2(sampleDirection, sampleRadiance);
// }
// radianceSH *= 1.0f / (NumSamples * SampleDirectionSphere_PDF());
//
// Example #2: calculating diffuse lighting for a surface from radiance projected onto L2 SH
//
// SH::L2 radianceSH = FetchRadianceSH(surfacePosition);
// float3 diffuseLighting = SH::CalculateIrradiance(radianceSH, surfaceNormal) * (diffuseAlbedo / Pi);
//
//=================================================================================================

#ifndef SH_HLSLI_
#define SH_HLSLI_

namespace SH
{

// Constants
static const float Pi = 3.141592654f;
static const float SqrtPi = sqrt(Pi);

static const float CosineA0 = Pi;
static const float CosineA1 = (2.0f * Pi) / 3.0f;
static const float CosineA2 = (0.25f * Pi);

static const float BasisL0 = 1 / (2 * SqrtPi);
static const float BasisL1 = sqrt(3) / (2 * SqrtPi);
static const float BasisL2_MN2 = sqrt(15) / (2 * SqrtPi);
static const float BasisL2_MN1 = sqrt(15) / (2 * SqrtPi);
static const float BasisL2_M0 = sqrt(5) / (4 * SqrtPi);
static const float BasisL2_M1 = sqrt(15) / (2 * SqrtPi);
static const float BasisL2_M2 = sqrt(15) / (4 * SqrtPi);

// L1 SH, 4 coeffients
template<typename T = float> struct L1
{
    T C[4];

    static L1<T> Zero()
    {
        return (L1<T>)0;
    }

    L1<T> operator+(L1<T> other)
    {
        L1<T> result;
        for(uint i = 0; i < 4; ++i)
            result.C[i] = C[i] + other.C[i];
        return result;
    }

    L1<T> operator-(L1<T> other)
    {
        L1<T> result;
        for(uint i = 0; i < 4; ++i)
            result.C[i] = C[i] - other.C[i];
        return result;
    }

    L1<T> operator*(T value)
    {
        L1<T> result;
        for(uint i = 0; i < 4; ++i)
            result.C[i] = C[i] * value;
        return result;
    }

    L1<T> operator/(T value)
    {
        L1<T> result;
        for(uint i = 0; i < 4; ++i)
            result.C[i] = C[i] / value;
        return result;
    }
};

typedef L1<half> L1_F16;
typedef L1<float3> L1_RGB;
typedef L1<half3> L1_F16_RGB;

// L2 SH, 9 coeffients
template<typename T = float> struct L2
{
    T C[9];

    static L2<T> Zero()
    {
        return (L2<T>)0;
    }

    L2<T> operator+(L2<T> other)
    {
        L2<T> result;
        for(uint i = 0; i < 9; ++i)
            result.C[i] = C[i] + other.C[i];
        return result;
    }

    L2<T> operator-(L2<T> other)
    {
        L2<T> result;
        for(uint i = 0; i < 9; ++i)
            result.C[i] = C[i] - other.C[i];
        return result;
    }

    L2<T> operator*(T value)
    {
        L2<T> result;
        for(uint i = 0; i < 9; ++i)
            result.C[i] = C[i] * value;
        return result;
    }

    L2<T> operator/(T value)
    {
        L2<T> result;
        for(uint i = 0; i < 9; ++i)
            result.C[i] = C[i] / value;
        return result;
    }
};

typedef L2<half> L2_F16;
typedef L2<float3> L2_RGB;
typedef L2<half3> L2_F16_RGB;

// Truncates a set of L2 coefficients to produce a set of L1 coefficients
template<typename T> L1<T> L2toL1(L2<T> sh)
{
    L1<T> result;
    for(uint i = 0; i < 4; ++i)
        result.C[i] = sh.C[i];
    return result;
}

template<typename T, typename TLerp> L1<T> Lerp(L1<T> x, L1<T> y, TLerp s)
{
    return x * ((TLerp)1.0 - s) + y * s;
}

template<typename T, typename TLerp> L2<T> Lerp(L2<T> x, L2<T> y, TLerp s)
{
    return x * ((TLerp)1.0 - s) + y * s;
}

// Projects a value in a single direction onto a set of L1 SH coefficients
template<typename T> L1<T> ProjectOntoL1(float3 direction, T value)
{
    L1<T> sh;

    // L0
    sh.C[0] = (T)(BasisL0 * value);

    // L1
    sh.C[1] = (T)(BasisL1 * direction.y) * value;
    sh.C[2] = (T)(BasisL1 * direction.z) * value;
    sh.C[3] = (T)(BasisL1 * direction.x) * value;

    return sh;
}

// Projects a value in a single direction onto a set of L2 SH coefficients
template<typename T> L2<T> ProjectOntoL2(float3 direction, T value)
{
    L2<T> sh;

    // L0
    sh.C[0] = (T)(BasisL0 * value);

    // L1
    sh.C[1] = (T)(BasisL1 * direction.y) * value;
    sh.C[2] = (T)(BasisL1 * direction.z) * value;
    sh.C[3] = (T)(BasisL1 * direction.x) * value;

    // L2
    sh.C[4] = (T)(BasisL2_MN2 * direction.x * direction.y) * value;
    sh.C[5] = (T)(BasisL2_MN1 * direction.y * direction.z) * value;
    sh.C[6] = (T)(BasisL2_M0 * (3.0f * direction.z * direction.z - 1.0f)) * value;
    sh.C[7] = (T)(BasisL2_M1 * direction.x * direction.z) * value;
    sh.C[8] = (T)(BasisL2_M2 * (direction.x * direction.x - direction.y * direction.y)) * value;

    return sh;
}

// Calculates the dot product of two sets of L1 SH coefficients
template<typename T> T DotProduct(L1<T> a, L1<T> b)
{
    T result = (T)0.0;
    for(uint i = 0; i < 4; ++i)
        result += a.C[i] * b.C[i];

    return result;
}

// Calculates the dot product of two sets of L2 SH coefficients
template<typename T> T DotProduct(L2<T> a, L2<T> b)
{
    T result = (T)0.0;
    for(uint i = 0; i < 9; ++i)
        result += a.C[i] * b.C[i];

    return result;
}

// Projects a delta in a direction onto SH and calculates the dot product with a set of L1 SH coefficients.
// Can be used to "look up" a value from SH coefficients in a particular direction.
template<typename T> T Evaluate(L1<T> sh, float3 direction)
{
    L1<T> projectedDelta = ProjectOntoL1(direction, (T)1.0);
    return DotProduct(projectedDelta, sh);
}

// Projects a delta in a direction onto SH and calculates the dot product with a set of L2 SH coefficients.
// Can be used to "look up" a value from SH coefficients in a particular direction.
template<typename T> T Evaluate(L2<T> sh, float3 direction)
{
    L2<T> projectedDelta = ProjectOntoL2(direction, (T)1.0);
    return DotProduct(projectedDelta, sh);
}

// Convolves a set of L1 SH coefficients with a set of L1 zonal harmonics
template<typename T> L1<T> ConvolveWithZH(L1<T> sh, float2 zh)
{
    // L0
    sh.C[0] *= zh.x;

    // L1
    sh.C[1] *= zh.y;
    sh.C[2] *= zh.y;
    sh.C[3] *= zh.y;

    return sh;
}

// Convolves a set of L2 SH coefficients with a set of L2 zonal harmonics
template<typename T> L1<T> ConvolveWithZH(L1<T> sh, float3 zh)
{
    // L0
    sh.C[0] *= zh.x;

    // L1
    sh.C[1] *= zh.y;
    sh.C[2] *= zh.y;
    sh.C[3] *= zh.y;

    // L2
    sh.C[4] *= zh.z;
    sh.C[5] *= zh.z;
    sh.C[6] *= zh.z;
    sh.C[7] *= zh.z;
    sh.C[8] *= zh.z;

    return sh;
}

// Convolves a set of L1 SH coefficients with a cosine lobe. See [2]
template<typename T> L1<T> ConvolveWithCosineLobe(L1<T> sh)
{
    return ConvolveWithZH(sh, float2(CosineA0, CosineA1));
}

// Convolves a set of L2 SH coefficients with a cosine lobe. See [2]
template<typename T> L2<T> ConvolveWithCosineLobe(L2<T> sh)
{
    return ConvolveWithZH(sh, float3(CosineA0, CosineA1, CosineA2));
}

// Computes the "optimal linear direction" for a set of SH coefficients, AKA the "dominant" direction. See [0].
template<typename T> float3 OptimalLinearDirection(L1<T> sh)
{
    float x = sh.C[3].x;
    float y = sh.C[1].x;
    float z = sh.C[2].x;
    return normalize(float3(x, y, z));
}

// Computes the direction and color of a directional light that approximates a set of L1 SH coefficients. See [0].
template<typename T> void ApproximateDirectionalLight(L1<T> sh, out float3 direction, out T color)
{
    direction = OptimalLinearDirection(sh);
    L1<T> dirSH = ProjectOntoL1(direction, (T)1.0);
    dirSH.C[0] = (T)0.0;
    color = DotProduct(dirSH, sh) * (T)(867.0f / (316.0f * Pi));
}

// Calculates the irradiance from a set of SH coefficients containing projected radiance.
// Convolves the radiance with a cosine lobe, and then evaluates the result in the given normal direction.
// Note that this does not scale the irradiance by 1 / Pi: if using this result for Lambertian diffuse,
// you will want to include the divide-by-pi that's part of the Lambertian BRDF.
// For example: float3 diffuse = CalculateIrradiance(sh, normal) * diffuseAlbedo / Pi;
template<typename T> T CalculateIrradiance(L1<T> sh, float3 normal)
{
    L1<T> convolved = ConvolveWithCosineLobe(sh);
    return Evaluate(convolved, normal);
}

// Calculates the irradiance from a set of SH coefficients containing projected radiance.
// Convolves the radiance with a cosine lobe, and then evaluates the result in the given normal direction.
// Note that this does not scale the irradiance by 1 / Pi: if using this result for Lambertian diffuse,
// you will want to include the divide-by-pi that's part of the Lambertian BRDF.
// For example: float3 diffuse = CalculateIrradiance(sh, normal) * diffuseAlbedo / Pi;
template<typename T> T CalculateIrradiance(L2<T> sh, float3 normal)
{
    L2<T> convolved = ConvolveWithCosineLobe(sh);
    return Evaluate(convolved, normal);
}

// Evaluates the irradiance from a set of L1 SH coeffecients using the non-linear fit from [1]
// Note that this does not scale the irradiance by 1 / Pi: if using this result for Lambertian diffuse,
// you will want to include the divide-by-pi that's part of the Lambertian BRDF.
// For example: float3 diffuse = CalculateIrradianceGeomerics(sh, normal) * diffuseAlbedo / Pi;
template<typename T, int N> vector<T, N> CalculateIrradianceGeomerics(L1< vector<T, N> > sh, float3 normal)
{
    vector<T, N> result = 0.0f;

    for(uint i = 0; i < N; ++i)
    {
        vector<T, 1> R0 = max(sh.C[0][i], vector<T, 1>(0.00001));

        vector<T, 3> R1 = vector<T, 1>(0.5) * vector<T, 3>(sh.C[3][i], sh.C[1][i], sh.C[2][i]);
        vector<T, 1> lenR1 = max(length(R1), vector<T, 1>(0.00001));

        vector<T, 1> q = vector<T, 1>(0.5) * (vector<T, 1>(1.0) + dot(R1 / lenR1, normal));

        vector<T, 1> p = vector<T, 1>(1.0) + vector<T, 1>(2.0) * lenR1 / R0;
        vector<T, 1> a = (vector<T, 1>(1.0) - lenR1 / R0) / (vector<T, 1>(1.0) + lenR1 / R0);

        result[i] = R0 * (a + (vector<T, 1>(1.0) - a) * (p + vector<T, 1>(1.0)) * pow(abs(q), p));
    }

    return result;
}

// Rotates a set of L1 coefficients by a rotation matrix
template<typename T> L1<T> Rotate(L1<T> sh, float3x3 rotation)
{
    const float r00 = rotation._m00;
    const float r10 = rotation._m01;
    const float r20 = -rotation._m02;

    const float r01 = rotation._m10;
    const float r11 = rotation._m11;
    const float r21 = -rotation._m12;

    const float r02 = -rotation._m20;
    const float r12 = -rotation._m21;
    const float r22 = rotation._m22;

    L1<T> result;

    // L0
    result.C[0] = sh.C[0];

    // L1
    result.C[1] = T(r11 * sh.C[1] - r12 * sh.C[2] + r10 * sh.C[3]);
    result.C[2] = T(-r21 * sh.C[1] + r22 * sh.C[2] - r20 * sh.C[3]);
    result.C[3] = T(r01 * sh.C[1] - r02 * sh.C[2] + r00 * sh.C[3]);

    return result;
}

// Rotates a set of L2 coefficients by a rotation matrix
template<typename T> L2<T> Rotate(L2<T> sh, float3x3 rotation)
{
    const float r00 = rotation._m00;
    const float r10 = rotation._m01;
    const float r20 = -rotation._m02;

    const float r01 = rotation._m10;
    const float r11 = rotation._m11;
    const float r21 = -rotation._m12;

    const float r02 = -rotation._m20;
    const float r12 = -rotation._m21;
    const float r22 = rotation._m22;

    L2<T> result;

    // L0
    result.C[0] = sh.C[0];

    // L1
    result.C[1] = T(r11 * sh.C[1] - r12 * sh.C[2] + r10 * sh.C[3]);
    result.C[2] = T(-r21 * sh.C[1] + r22 * sh.C[2] - r20 * sh.C[3]);
    result.C[3] = T(r01 * sh.C[1] - r02 * sh.C[2] + r00 * sh.C[3]);

    // L2
    const float t41 = r01 * r00;
    const float t43 = r11 * r10;
    const float t48 = r11 * r12;
    const float t50 = r01 * r02;
    const float t55 = r02 * r02;
    const float t57 = r22 * r22;
    const float t58 = r12 * r12;
    const float t61 = r00 * r02;
    const float t63 = r10 * r12;
    const float t68 = r10 * r10;
    const float t70 = r01 * r01;
    const float t72 = r11 * r11;
    const float t74 = r00 * r00;
    const float t76 = r21 * r21;
    const float t78 = r20 * r20;

    const float v173 = 0.1732050808e1f;
    const float v577 = 0.5773502693e0f;
    const float v115 = 0.1154700539e1f;
    const float v288 = 0.2886751347e0f;
    const float v866 = 0.8660254040e0f;

    float r[25];
    r[0] = r11 * r00 + r01 * r10;
    r[1] = -r01 * r12 - r11 * r02;
    r[2] =  v173 * r02 * r12;
    r[3] = -r10 * r02 - r00 * r12;
    r[4] = r00 * r10 - r01 * r11;
    r[5] = - r11 * r20 - r21 * r10;
    r[6] = r11 * r22 + r21 * r12;
    r[7] = -v173 * r22 * r12;
    r[8] = r20 * r12 + r10 * r22;
    r[9] = -r10 * r20 + r11 * r21;
    r[10] = -v577 * (t41 + t43) + v115 * r21 * r20;
    r[11] = v577 * (t48 + t50) - v115 * r21 * r22;
    r[12] = -0.5f * (t55 + t58) + t57;
    r[13] = v577 * (t61 + t63) - v115 * r20 * r22;
    r[14] =  v288 * (t70 - t68 + t72 - t74) - v577 * (t76 - t78);
    r[15] = -r01 * r20 -  r21 * r00;
    r[16] = r01 * r22 + r21 * r02;
    r[17] = -v173 * r22 * r02;
    r[18] = r00 * r22 + r20 * r02;
    r[19] = -r00 * r20 + r01 * r21;
    r[20] = t41 - t43;
    r[21] = -t50 + t48;
    r[22] =  v866 * (t55 - t58);
    r[23] = t63 - t61;
    r[24] = 0.5f * (t74 - t68 - t70 +  t72);

    for(uint i = 0; i < 5; ++i)
    {
        const uint base = i * 5;
        result.C[4 + i] = T(r[base + 0] * sh.C[4] + r[base + 1] * sh.C[5] +
                            r[base + 2] * sh.C[6] + r[base + 3] * sh.C[7] +
                            r[base + 4] * sh.C[8]);
    }

    return result;
}

} // namespace SH

// References:
//
// [0] Stupid SH Tricks by Peter-Pike Sloan - https://www.ppsloan.org/publications/StupidSH36.pdf
// [1] Converting SH Radiance to Irradiance by Graham Hazel - https://grahamhazel.com/blog/2017/12/22/converting-sh-radiance-to-irradiance/
// [2] An Efficient Representation for Irradiance Environment Maps by Ravi Ramamoorthi and Pat Hanrahan - https://cseweb.ucsd.edu/~ravir/6998/papers/envmap.pdf

#endif // SH_HLSLI_