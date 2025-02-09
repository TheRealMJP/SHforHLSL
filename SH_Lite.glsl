//=================================================================================================
//
//  SHforHLSL - Spherical harmonics suppport library for HLSL, by MJP
//  https://github.com/TheRealMJP/SHforHLSL
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

//=================================================================================================
//
// This header is intended to be included directly from HLSL code, or similar. It is a simplified
// version of SH.hlsli, and does not use templates, operator overloads, or other HLSL 2021+
// features. It implements types and utility functions for working with low-order spherical
// harmonics, focused on use cases for graphics.
//
// Currently this library has support for SH_L1 (2 bands, 4 coefficients) and
// SH_L2 (3 bands, 9 coefficients) SH. Depending on the author and material you're reading, you may
// see SH_L1 referred to as both first-order or second-order, and SH_L2 referred to as second-order
// or third-order. Ravi Ramamoorthi tends to refer to three bands as second-order, and
// Peter-Pike Sloan tends to refer to three bands as third-order. This library always uses SH_L1 and
// SH_L2 for clarity.
//
// The core SH types all use 32-bit floats for storing coefficients, with either 1 scalar or 3
// floats for separate RGB coefficients.
//
// Example #1: integrating and projecting radiance onto SH_L2 SH
//
// SH_L2_RGB radianceSH = SH_L2_RGB_Zero();
// for(int32_t sampleIndex = 0; sampleIndex < NumSamples; ++sampleIndex)
// {
//     vec2 u1u2 = RandomFloat2(sampleIndex, NumSamples);
//     vec3 sampleDirection = SampleDirectionSphere(u1u2);
//     vec3 sampleRadiance = CalculateIncomingRadiance(sampleDirection);
//     radianceSH += SH_ProjectOntoL2(sampleDirection, sampleRadiance);
// }
// radianceSH *= 1.0f / (NumSamples * SampleDirectionSphere_PDF());
//
// Example #2: calculating diffuse lighting for a surface from radiance projected onto SH_L2 SH
//
// SH_L2_RGB radianceSH = FetchRadianceSH(surfacePosition);
// vec3 diffuseLighting = SH_CalculateIrradiance(radianceSH, surfaceNormal) * (diffuseAlbedo / Pi);
//
//=================================================================================================

#ifndef SH_LITE_GLSL_
#define SH_LITE_GLSL_

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif
#ifndef M_SQRTPI
#define M_SQRTPI 1.77245385090551602730 /* sqrt(pi) */
#endif

const float SH_CosineA0 = M_PI;
const float SH_CosineA1 = (2.0 * M_PI) / 3.0;
const float SH_CosineA2 = (0.25 * M_PI);

const float SH_BasisL0 = 1 / (2 * M_SQRTPI);
const float SH_BasisL1 = sqrt(3) / (2 * M_SQRTPI);
const float SH_BasisL2_MN2 = sqrt(15) / (2 * M_SQRTPI);
const float SH_BasisL2_MN1 = sqrt(15) / (2 * M_SQRTPI);
const float SH_BasisL2_M0 = sqrt(5) / (4 * M_SQRTPI);
const float SH_BasisL2_M1 = sqrt(15) / (2 * M_SQRTPI);
const float SH_BasisL2_M2 = sqrt(15) / (4 * M_SQRTPI);

// Core SH types containing the coefficients
const uint SH_L1_NumCoefficients = 4;
struct SH_L1
{
    float C[SH_L1_NumCoefficients];
};
SH_L1 SH_L1_Zero() {
    return SH_L1(
        float[SH_L1_NumCoefficients](0.0, 0.0, 0.0, 0.0)
    );
}

struct SH_L1_RGB
{
    vec3 C[SH_L1_NumCoefficients];
};
SH_L1_RGB SH_L1_RGB_Zero() {
    return SH_L1_RGB(
        vec3[4](0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx)
    );
}

const uint SH_L2_NumCoefficients = 9;
struct SH_L2
{
    float C[SH_L2_NumCoefficients];
};
SH_L2 SH_L2_Zero()
{
    return SH_L2(
        float[9](0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
    );
}

struct SH_L2_RGB
{
    vec3 C[SH_L2_NumCoefficients];
};
SH_L2_RGB SH_L2_RGB_Zero()
{
    return SH_L2_RGB(
        vec3[9](0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx, 0.0.xxx)
    );
}

// Sum two sets of SH coefficients
SH_L1 SH_Add(SH_L1 a, SH_L1 b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] += b.C[i];
    return a;
}

SH_L1_RGB SH_Add(SH_L1_RGB a, SH_L1_RGB b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] += b.C[i];
    return a;
}

SH_L2 SH_Add(SH_L2 a, SH_L2 b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] += b.C[i];
    return a;
}

SH_L2_RGB SH_Add(SH_L2_RGB a, SH_L2_RGB b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] += b.C[i];
    return a;
}

// Substract two sets of SH coefficients
SH_L1 SH_Subtract(SH_L1 a, SH_L1 b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] -= b.C[i];
    return a;
}

SH_L1_RGB SH_Subtract(SH_L1_RGB a, SH_L1_RGB b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] -= b.C[i];
    return a;
}

SH_L2 SH_Subtract(SH_L2 a, SH_L2 b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] -= b.C[i];
    return a;
}

SH_L2_RGB SH_Subtract(SH_L2_RGB a, SH_L2_RGB b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] -= b.C[i];
    return a;
}

// Multiply a set of SH coefficients by a single value
SH_L1 SH_Multiply(SH_L1 a, float b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] *= b;
    return a;
}

SH_L1_RGB SH_Multiply(SH_L1_RGB a, vec3 b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] *= b;
    return a;
}

SH_L2 SH_Multiply(SH_L2 a, float b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] *= b;
    return a;
}

SH_L2_RGB SH_Multiply(SH_L2_RGB a, vec3 b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] *= b;
    return a;
}

// Divide a set of SH coefficients by a single value
SH_L1 SH_Divide(SH_L1 a, float b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] /= b;
    return a;
}

SH_L1_RGB SH_Divide(SH_L1_RGB a, vec3 b)
{
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        a.C[i] /= b;
    return a;
}

SH_L2 SH_Divide(SH_L2 a, float b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] /= b;
    return a;
}

SH_L2_RGB SH_Divide(SH_L2_RGB a, vec3 b)
{
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        a.C[i] /= b;
    return a;
}

// Truncates a set of SH_L2 coefficients to produce a set of SH_L1 coefficients
SH_L1 SH_L2toL1(SH_L2 sh)
{
    SH_L1 result;
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        result.C[i] = sh.C[i];
    return result;
}

SH_L1_RGB SH_L2toL1(SH_L2_RGB sh)
{
    SH_L1_RGB result;
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        result.C[i] = sh.C[i];
    return result;
}

// Converts from scalar to RGB SH coefficients
SH_L1_RGB SH_ToRGB(SH_L1 sh)
{
    SH_L1_RGB result;
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        result.C[i] = sh.C[i].xxx;
    return result;
}

SH_L2_RGB SH_ToRGB(SH_L2 sh)
{
    SH_L2_RGB result;
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        result.C[i] = sh.C[i].xxx;
    return result;
}

// Linear interpolation
SH_L1 SH_Mix(SH_L1 x, SH_L1 y, float s)
{
    return SH_Add(SH_Multiply(x, 1.0 - s), SH_Multiply(y, s));
}

SH_L1_RGB SH_Mix(SH_L1_RGB x, SH_L1_RGB y, float s)
{
    return SH_Add(SH_Multiply(x, vec3(1.0 - s)), SH_Multiply(y, s.xxx));
}

SH_L2 SH_Mix(SH_L2 x, SH_L2 y, float s)
{
    return SH_Add(SH_Multiply(x, 1.0 - s), SH_Multiply(y, s));
}

SH_L2_RGB SH_Mix(SH_L2_RGB x, SH_L2_RGB y, float s)
{
    return SH_Add(SH_Multiply(x, vec3(1.0 - s)), SH_Multiply(y, s.xxx));
}

// Projects a value in a single direction onto a set of SH_L1 SH coefficients
SH_L1 SH_ProjectOntoL1(vec3 direction, float value)
{
    SH_L1 sh;

    // L0
    sh.C[0] = SH_BasisL0 * value;

    // SH_L1
    sh.C[1] = SH_BasisL1 * direction.y * value;
    sh.C[2] = SH_BasisL1 * direction.z * value;
    sh.C[3] = SH_BasisL1 * direction.x * value;

    return sh;
}

SH_L1_RGB SH_ProjectOntoL1_RGB(vec3 direction, vec3 value)
{
    SH_L1_RGB sh;

    // L0
    sh.C[0] = SH_BasisL0 * value;

    // SH_L1
    sh.C[1] = SH_BasisL1 * direction.y * value;
    sh.C[2] = SH_BasisL1 * direction.z * value;
    sh.C[3] = SH_BasisL1 * direction.x * value;

    return sh;
}

// Projects a value in a single direction onto a set of SH_L2 SH coefficients
SH_L2 SH_ProjectOntoL2(vec3 direction, float value)
{
    SH_L2 sh;

    // L0
    sh.C[0] = SH_BasisL0 * value;

    // SH_L1
    sh.C[1] = SH_BasisL1 * direction.y * value;
    sh.C[2] = SH_BasisL1 * direction.z * value;
    sh.C[3] = SH_BasisL1 * direction.x * value;

    // SH_L2
    sh.C[4] = SH_BasisL2_MN2 * direction.x * direction.y * value;
    sh.C[5] = SH_BasisL2_MN1 * direction.y * direction.z * value;
    sh.C[6] = SH_BasisL2_M0 * (3.0 * direction.z * direction.z - 1.0) * value;
    sh.C[7] = SH_BasisL2_M1 * direction.x * direction.z * value;
    sh.C[8] = SH_BasisL2_M2 * (direction.x * direction.x - direction.y * direction.y) * value;

    return sh;
}

SH_L2_RGB SH_ProjectOntoL2_RGB(vec3 direction, vec3 value)
{
    SH_L2_RGB sh;

    // L0
    sh.C[0] = SH_BasisL0 * value;

    // SH_L1
    sh.C[1] = SH_BasisL1 * direction.y * value;
    sh.C[2] = SH_BasisL1 * direction.z * value;
    sh.C[3] = SH_BasisL1 * direction.x * value;

    // SH_L2
    sh.C[4] = SH_BasisL2_MN2 * direction.x * direction.y * value;
    sh.C[5] = SH_BasisL2_MN1 * direction.y * direction.z * value;
    sh.C[6] = SH_BasisL2_M0 * (3.0 * direction.z * direction.z - 1.0) * value;
    sh.C[7] = SH_BasisL2_M1 * direction.x * direction.z * value;
    sh.C[8] = SH_BasisL2_M2 * (direction.x * direction.x - direction.y * direction.y) * value;

    return sh;
}

// Calculates the dot product of two sets of SH_L1 SH coefficients
float SH_DotProduct(SH_L1 a, SH_L1 b)
{
    float result = 0.0;
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        result += a.C[i] * b.C[i];

    return result;
}

vec3 SH_DotProduct(SH_L1_RGB a, SH_L1_RGB b)
{
    vec3 result = 0.0.xxx;
    for(uint i = 0; i < SH_L1_NumCoefficients; ++i)
        result += a.C[i] * b.C[i];

    return result;
}

// Calculates the dot product of two sets of SH_L2 SH coefficients
float SH_DotProduct(SH_L2 a, SH_L2 b)
{
    float result = 0.0;
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        result += a.C[i] * b.C[i];

    return result;
}

vec3 SH_DotProduct(SH_L2_RGB a, SH_L2_RGB b)
{
    vec3 result = 0.0.xxx;
    for(uint i = 0; i < SH_L2_NumCoefficients; ++i)
        result += a.C[i] * b.C[i];

    return result;
}

// Projects a delta in a direction onto SH and calculates the dot product with a set of SH_L1 SH coefficients.
// Can be used to "look up" a value from SH coefficients in a particular direction.
float SH_Evaluate(SH_L1 sh, vec3 direction)
{
    SH_L1 projectedDelta = SH_ProjectOntoL1(direction, 1.0);
    return SH_DotProduct(projectedDelta, sh);
}

vec3 SH_Evaluate(SH_L1_RGB sh, vec3 direction)
{
    SH_L1_RGB projectedDelta = SH_ProjectOntoL1_RGB(direction, 1.0.xxx);
    return SH_DotProduct(projectedDelta, sh);
}

// Projects a delta in a direction onto SH and calculates the dot product with a set of SH_L2 SH coefficients.
// Can be used to "look up" a value from SH coefficients in a particular direction.
float SH_Evaluate(SH_L2 sh, vec3 direction)
{
    SH_L2 projectedDelta = SH_ProjectOntoL2(direction, 1.0);
    return SH_DotProduct(projectedDelta, sh);
}

vec3 SH_Evaluate(SH_L2_RGB sh, vec3 direction)
{
    SH_L2_RGB projectedDelta = SH_ProjectOntoL2_RGB(direction, 1.0.xxx);
    return SH_DotProduct(projectedDelta, sh);
}

// Convolves a set of SH_L1 SH coefficients with a set of SH_L1 zonal harmonics
SH_L1 SH_ConvolveWithZH(SH_L1 sh, vec2 zh)
{
    // L0
    sh.C[0] *= zh.x;

    // SH_L1
    sh.C[1] *= zh.y;
    sh.C[2] *= zh.y;
    sh.C[3] *= zh.y;

    return sh;
}

SH_L1_RGB SH_ConvolveWithZH(SH_L1_RGB sh, vec2 zh)
{
    // L0
    sh.C[0] *= zh.x;

    // SH_L1
    sh.C[1] *= zh.y;
    sh.C[2] *= zh.y;
    sh.C[3] *= zh.y;

    return sh;
}

// Convolves a set of SH_L2 SH coefficients with a set of SH_L2 zonal harmonics
SH_L2 SH_ConvolveWithZH(SH_L2 sh, vec3 zh)
{
    // L0
    sh.C[0] *= zh.x;

    // SH_L1
    sh.C[1] *= zh.y;
    sh.C[2] *= zh.y;
    sh.C[3] *= zh.y;

    // SH_L2
    sh.C[4] *= zh.z;
    sh.C[5] *= zh.z;
    sh.C[6] *= zh.z;
    sh.C[7] *= zh.z;
    sh.C[8] *= zh.z;

    return sh;
}

SH_L2_RGB SH_ConvolveWithZH(SH_L2_RGB sh, vec3 zh)
{
    // L0
    sh.C[0] *= zh.x;

    // SH_L1
    sh.C[1] *= zh.y;
    sh.C[2] *= zh.y;
    sh.C[3] *= zh.y;

    // SH_L2
    sh.C[4] *= zh.z;
    sh.C[5] *= zh.z;
    sh.C[6] *= zh.z;
    sh.C[7] *= zh.z;
    sh.C[8] *= zh.z;

    return sh;
}

// Convolves a set of SH_L1 SH coefficients with a cosine lobe. See [2]
SH_L1 SH_ConvolveWithCosineLobe(SH_L1 sh)
{
    return SH_ConvolveWithZH(sh, vec2(SH_CosineA0, SH_CosineA1));
}

SH_L1_RGB SH_ConvolveWithCosineLobe(SH_L1_RGB sh)
{
    return SH_ConvolveWithZH(sh, vec2(SH_CosineA0, SH_CosineA1));
}

// Convolves a set of SH_L2 SH coefficients with a cosine lobe. See [2]
SH_L2 SH_ConvolveWithCosineLobe(SH_L2 sh)
{
    return SH_ConvolveWithZH(sh, vec3(SH_CosineA0, SH_CosineA1, SH_CosineA2));
}

SH_L2_RGB SH_ConvolveWithCosineLobe(SH_L2_RGB sh)
{
    return SH_ConvolveWithZH(sh, vec3(SH_CosineA0, SH_CosineA1, SH_CosineA2));
}

// Computes the "optimal linear direction" for a set of SH coefficients, AKA the "dominant" direction. See [0].
vec3 SH_OptimalLinearDirection(SH_L1 sh)
{
    return normalize(vec3(sh.C[3], sh.C[1], sh.C[2]));
}

vec3 SH_OptimalLinearDirection(SH_L1_RGB sh)
{
    vec3 direction = 0.0.xxx;
    for(uint i = 0; i < 3; ++i)
    {
        direction.x += sh.C[3][i];
        direction.y += sh.C[1][i];
        direction.z += sh.C[2][i];
    }
    return normalize(direction);
}

// Computes the direction and color of a directional light that approximates a set of SH_L1 SH coefficients. See [0].
void SH_ApproximateDirectionalLight(SH_L1 sh, out vec3 direction, out float intensity)
{
    direction = SH_OptimalLinearDirection(sh);
    SH_L1 dirSH = SH_ProjectOntoL1(direction, 1.0);
    dirSH.C[0] = 0.0;
    intensity = SH_DotProduct(dirSH, sh) * (867.0 / (316.0 * M_PI));
}

void SH_ApproximateDirectionalLight(SH_L1_RGB sh, out vec3 direction, out vec3 color)
{
    direction = SH_OptimalLinearDirection(sh);
    SH_L1_RGB dirSH = SH_ProjectOntoL1_RGB(direction, 1.0.xxx);
    dirSH.C[0] = 0.0.xxx;
    color = SH_DotProduct(dirSH, sh) * (867.0 / (316.0 * M_PI));
}

// Calculates the irradiance from a set of SH coefficients containing projected radiance.
// Convolves the radiance with a cosine lobe, and then evaluates the result in the given normal direction.
// Note that this does not scale the irradiance by 1 / Pi: if using this result for Lambertian diffuse,
// you will want to include the divide-by-pi that's part of the Lambertian BRDF.
// For example: vec3 diffuse = CalculateIrradiance(sh, normal) * diffuseAlbedo / Pi;
float SH_CalculateIrradiance(SH_L1 sh, vec3 normal)
{
    SH_L1 convolved = SH_ConvolveWithCosineLobe(sh);
    return SH_Evaluate(convolved, normal);
}

vec3 SH_CalculateIrradiance(SH_L1_RGB sh, vec3 normal)
{
    SH_L1_RGB convolved = SH_ConvolveWithCosineLobe(sh);
    return SH_Evaluate(convolved, normal);
}

// Calculates the irradiance from a set of SH coefficients containing projected radiance.
// Convolves the radiance with a cosine lobe, and then evaluates the result in the given normal direction.
// Note that this does not scale the irradiance by 1 / Pi: if using this result for Lambertian diffuse,
// you will want to include the divide-by-pi that's part of the Lambertian BRDF.
// For example: vec3 diffuse = CalculateIrradiance(sh, normal) * diffuseAlbedo / Pi;
float SH_CalculateIrradiance(SH_L2 sh, vec3 normal)
{
    SH_L2 convolved = SH_ConvolveWithCosineLobe(sh);
    return SH_Evaluate(convolved, normal);
}

vec3 SH_CalculateIrradiance(SH_L2_RGB sh, vec3 normal)
{
    SH_L2_RGB convolved = SH_ConvolveWithCosineLobe(sh);
    return SH_Evaluate(convolved, normal);
}

// Calculates the irradiance from a set of SH_L1 SH coeffecients using the non-linear fit from [1]
// Note that this does not scale the irradiance by 1 / Pi: if using this result for Lambertian diffuse,
// you will want to include the divide-by-pi that's part of the Lambertian BRDF.
// For example: vec3 diffuse = CalculateIrradianceGeomerics(sh, normal) * diffuseAlbedo / Pi;
float SH_CalculateIrradianceGeomerics(SH_L1 sh, vec3 normal)
{
    float R0 = max(sh.C[0], 0.00001);

    vec3 R1 = 0.5 * vec3(sh.C[3], sh.C[1], sh.C[2]);
    float lenR1 = max(length(R1), 0.00001);

    float q = 0.5 * (1.0 + dot(R1 / lenR1, normal));

    float p = 1.0 + 2.0 * lenR1 / R0;
    float a = (1.0 - lenR1 / R0) / (1.0 + lenR1 / R0);

    return R0 * (a + (1.0 - a) * (p + 1.0) * pow(abs(q), p));
}

vec3 SH_CalculateIrradianceGeomerics(SH_L1_RGB sh, vec3 normal)
{
    SH_L1 shr = { { sh.C[0].x, sh.C[1].x, sh.C[2].x, sh.C[3].x } };
    SH_L1 shg = { { sh.C[0].y, sh.C[1].y, sh.C[2].y, sh.C[3].y } };
    SH_L1 shb = { { sh.C[0].z, sh.C[1].z, sh.C[2].z, sh.C[3].z } };

    return vec3(SH_CalculateIrradianceGeomerics(shr, normal), SH_CalculateIrradianceGeomerics(shg, normal), SH_CalculateIrradianceGeomerics(shb, normal));
}

// Calculates the irradiance from a set of SH_L1 SH coefficientions by 'hallucinating" L3 zonal harmonics. See [4].
float SH_CalculateIrradianceL1ZH3Hallucinate(SH_L1 sh, vec3 normal)
{
    const vec3 zonalAxis = normalize(vec3(sh.C[3], sh.C[1], sh.C[2]));

    float ratio = abs(dot(vec3(sh.C[3], sh.C[1], sh.C[2]), zonalAxis)) / sh.C[0];

    const float zonalL2Coeff = sh.C[0] * (0.08 * ratio + 0.6 * ratio * ratio);

    const float fZ = dot(zonalAxis, normal);
    const float zhDir = sqrt(5.0 / (16.0 * M_PI)) * (3.0 * fZ * fZ - 1.0);

    const float baseIrradiance = SH_CalculateIrradiance(sh, normal);

    return baseIrradiance + ((M_PI * 0.25) * zonalL2Coeff * zhDir);
}

vec3 SH_CalculateIrradianceL1ZH3Hallucinate(SH_L1_RGB sh, vec3 normal)
{
    const vec3 lumCoefficients = vec3(0.2126, 0.7152, 0.0722);
    const vec3 zonalAxis = normalize(vec3(dot(sh.C[3], lumCoefficients), dot(sh.C[1], lumCoefficients), dot(sh.C[2], lumCoefficients)));

    vec3 ratio;
    for(uint i = 0; i < 3; ++i)
        ratio[i] = abs(dot(vec3(sh.C[3][i], sh.C[1][i], sh.C[2][i]), zonalAxis)) / sh.C[0][i];

    const vec3 zonalL2Coeff = sh.C[0] * (0.08 * ratio + 0.6 * ratio * ratio);

    const float fZ = dot(zonalAxis, normal);
    const float zhDir = sqrt(5.0 / (16.0 * M_PI)) * (3.0 * fZ * fZ - 1.0);

    const vec3 baseIrradiance = SH_CalculateIrradiance(sh, normal);

    return baseIrradiance + ((M_PI * 0.25) * zonalL2Coeff * zhDir);
}

// Approximates a GGX lobe with a given roughness/alpha as SH_L1 zonal harmonics, using a fitted curve
vec2 SH_ApproximateGGXAsL1ZH(float ggxAlpha)
{
    const float l1Scale = 1.66711256633276 / (1.65715038133932 + ggxAlpha);
    return vec2(1.0, l1Scale);
}

// Approximates a GGX lobe with a given roughness/alpha as SH_L2 zonal harmonics, using a fitted curve
vec3 SH_ApproximateGGXAsL2ZH(float ggxAlpha)
{
    const float l1Scale = 1.66711256633276 / (1.65715038133932 + ggxAlpha);
    const float l2Scale = 1.56127990596116 / (0.96989757593282 + ggxAlpha) - 0.599972342361123;
    return vec3(1.0, l1Scale, l2Scale);
}

// Convolves a set of SH_L1 SH coefficients with a GGX lobe for a given roughness/alpha
SH_L1 SH_ConvolveWithGGX(SH_L1 sh, float ggxAlpha)
{
    return SH_ConvolveWithZH(sh, SH_ApproximateGGXAsL1ZH(ggxAlpha));
}

SH_L1_RGB SH_ConvolveWithGGX(SH_L1_RGB sh, float ggxAlpha)
{
    return SH_ConvolveWithZH(sh, SH_ApproximateGGXAsL1ZH(ggxAlpha));
}

// Convolves a set of SH_L2 SH coefficients with a GGX lobe for a given roughness/alpha
SH_L2 SH_ConvolveWithGGX(SH_L2 sh, float ggxAlpha)
{
    return SH_ConvolveWithZH(sh, SH_ApproximateGGXAsL2ZH(ggxAlpha));
}

SH_L2_RGB SH_ConvolveWithGGX(SH_L2_RGB sh, float ggxAlpha)
{
    return SH_ConvolveWithZH(sh, SH_ApproximateGGXAsL2ZH(ggxAlpha));
}

// Given a set of SH_L1 SH coefficients represnting incoming radiance, determines a directional light
// direction, color, and modified roughness value that can be used to compute an approximate specular term. See [5]
void SH_ExtractSpecularDirLight(SH_L1 shRadiance, float sqrtRoughness, out vec3 lightDir, out float lightIntensity, out float modifiedSqrtRoughness)
{
    vec3 avgL1 = vec3(shRadiance.C[3], shRadiance.C[1], shRadiance.C[2]);
    avgL1 *= 0.5;
    float avgL1len = length(avgL1);

    lightDir = avgL1 / avgL1len;
    lightIntensity = SH_Evaluate(shRadiance, lightDir) * M_PI;
    modifiedSqrtRoughness = clamp(sqrtRoughness / sqrt(avgL1len), 0.0, 1.0);
}

void SH_ExtractSpecularDirLight(SH_L1_RGB shRadiance, float sqrtRoughness, out vec3 lightDir, out vec3 lightColor, out float modifiedSqrtRoughness)
{
    vec3 avgL1 = vec3(dot(shRadiance.C[3] / shRadiance.C[0], 0.33333333.xxx), dot(shRadiance.C[1] / shRadiance.C[0], 0.33333333.xxx), dot(shRadiance.C[2] / shRadiance.C[0], 0.33333333.xxx));
    avgL1 *= 0.5;
    float avgL1len = length(avgL1);

    lightDir = avgL1 / avgL1len;
    lightColor = SH_Evaluate(shRadiance, lightDir) * M_PI;
    modifiedSqrtRoughness = clamp(sqrtRoughness / sqrt(avgL1len), 0.0, 1.0);
}

// Rotates a set of SH_L1 coefficients by a rotation matrix. Adapted from DirectX_XMSHRotate [3]
SH_L1 SH_Rotate(SH_L1 sh, mat3 rotation)
{
    const float r00 = rotation[0][0];
    const float r10 = rotation[1][0];
    const float r20 = -rotation[2][0];

    const float r01 = rotation[0][1];
    const float r11 = rotation[1][1];
    const float r21 = -rotation[2][1];

    const float r02 = -rotation[0][2];
    const float r12 = -rotation[1][2];
    const float r22 = rotation[2][2];

    SH_L1 result;

    // L0
    result.C[0] = sh.C[0];

    // L1
    result.C[1] = (r11 * sh.C[1] - r12 * sh.C[2] + r10 * sh.C[3]);
    result.C[2] = (-r21 * sh.C[1] + r22 * sh.C[2] - r20 * sh.C[3]);
    result.C[3] = (r01 * sh.C[1] - r02 * sh.C[2] + r00 * sh.C[3]);

    return result;
}
SH_L1 SH_Rotate(mat3 rotation, SH_L1 sh)
{
    return SH_Rotate(sh, transpose(rotation));
}

SH_L1_RGB SH_Rotate(SH_L1_RGB sh, mat3 rotation)
{
    const float r00 = rotation[0][0];
    const float r10 = rotation[1][0];
    const float r20 = -rotation[2][0];

    const float r01 = rotation[0][1];
    const float r11 = rotation[1][1];
    const float r21 = -rotation[2][1];

    const float r02 = -rotation[0][2];
    const float r12 = -rotation[1][2];
    const float r22 = rotation[2][2];

    SH_L1_RGB result;

    // L0
    result.C[0] = sh.C[0];

    // L1
    result.C[1] = (r11 * sh.C[1] - r12 * sh.C[2] + r10 * sh.C[3]);
    result.C[2] = (-r21 * sh.C[1] + r22 * sh.C[2] - r20 * sh.C[3]);
    result.C[3] = (r01 * sh.C[1] - r02 * sh.C[2] + r00 * sh.C[3]);

    return result;
}
SH_L1_RGB SH_Rotate(mat3 rotation, SH_L1_RGB sh)
{
    return SH_Rotate(sh, transpose(rotation));
}

// Rotates a set of SH_L2 coefficients by a rotation matrix. Adapted from DirectX::XMSHRotate [3]
SH_L2 SH_Rotate(SH_L2 sh, mat3 rotation)
{
    const float r00 = rotation[0][0];
    const float r10 = rotation[1][0];
    const float r20 = -rotation[2][0];

    const float r01 = rotation[0][1];
    const float r11 = rotation[1][1];
    const float r21 = -rotation[2][1];

    const float r02 = -rotation[0][2];
    const float r12 = -rotation[1][2];
    const float r22 = rotation[2][2];

    SH_L2 result;

    // L0
    result.C[0] = sh.C[0];

    // SH_L1
    result.C[1] = (r11 * sh.C[1] - r12 * sh.C[2] + r10 * sh.C[3]);
    result.C[2] = (-r21 * sh.C[1] + r22 * sh.C[2] - r20 * sh.C[3]);
    result.C[3] = (r01 * sh.C[1] - r02 * sh.C[2] + r00 * sh.C[3]);

    // SH_L2
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

    const float v173 = 0.1732050808e1;
    const float v577 = 0.5773502693e0;
    const float v115 = 0.1154700539e1;
    const float v288 = 0.2886751347e0;
    const float v866 = 0.8660254040e0;

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
    r[12] = -0.5 * (t55 + t58) + t57;
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
    r[24] = 0.5 * (t74 - t68 - t70 +  t72);

    for(uint i = 0; i < 5; ++i)
    {
        const uint base = i * 5;
        result.C[4 + i] = (r[base + 0] * sh.C[4] + r[base + 1] * sh.C[5] +
                           r[base + 2] * sh.C[6] + r[base + 3] * sh.C[7] +
                           r[base + 4] * sh.C[8]);
    }

    return result;
}
SH_L2 SH_Rotate(mat3 rotation, SH_L2 sh)
{
    return SH_Rotate(sh, transpose(rotation));
}

SH_L2_RGB SH_Rotate(SH_L2_RGB sh, mat3 rotation)
{
    const float r00 = rotation[0][0];
    const float r10 = rotation[1][0];
    const float r20 = -rotation[2][0];

    const float r01 = rotation[0][1];
    const float r11 = rotation[1][1];
    const float r21 = -rotation[2][1];

    const float r02 = -rotation[2][0];
    const float r12 = -rotation[2][1];
    const float r22 = rotation[2][2];

    SH_L2_RGB result;

    // L0
    result.C[0] = sh.C[0];

    // L1
    result.C[1] = (r11 * sh.C[1] - r12 * sh.C[2] + r10 * sh.C[3]);
    result.C[2] = (-r21 * sh.C[1] + r22 * sh.C[2] - r20 * sh.C[3]);
    result.C[3] = (r01 * sh.C[1] - r02 * sh.C[2] + r00 * sh.C[3]);

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

    const float v173 = 0.1732050808e1;
    const float v577 = 0.5773502693e0;
    const float v115 = 0.1154700539e1;
    const float v288 = 0.2886751347e0;
    const float v866 = 0.8660254040e0;

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
    r[12] = -0.5 * (t55 + t58) + t57;
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
    r[24] = 0.5 * (t74 - t68 - t70 +  t72);

    for(uint i = 0; i < 5; ++i)
    {
        const uint base = i * 5;
        result.C[4 + i] = (r[base + 0] * sh.C[4] + r[base + 1] * sh.C[5] +
                           r[base + 2] * sh.C[6] + r[base + 3] * sh.C[7] +
                           r[base + 4] * sh.C[8]);
    }

    return result;
}
SH_L2_RGB SH_Rotate(mat3 rotation, SH_L2_RGB sh)
{
    return SH_Rotate(sh, transpose(rotation));
}

// References:
//
// [0] Stupid SH Tricks by Peter-Pike Sloan - https://www.ppsloan.org/publications/StupidSH36.pdf
// [1] Converting SH Radiance to Irradiance by Graham Hazel - https://grahamhazel.com/blog/2017/12/22/converting-sh-radiance-to-irradiance/
// [2] An Efficient Representation for Irradiance Environment Maps by Ravi Ramamoorthi and Pat Hanrahan - https://cseweb.ucsd.edu/~ravir/6998/papers/envmap.pdf
// [3] SHMath by Chuck Walbourn (originally written by Peter-Pike Sloan) - https://walbourn.github.io/spherical-harmonics-math/
// [4] ZH3: Quadratic Zonal Harmonics by Thomas Roughton, Peter-Pike Sloan, Ari Silvennoinen, Michal Iwanicki, and Peter Shirley - https://torust.me/ZH3.pdf
// [5] Precomputed Global Illumination in Frostbite by Yuriy O'Donnell - https://www.ea.com/frostbite/news/precomputed-global-illumination-in-frostbite

#endif // SH_LITE_GLSL_
