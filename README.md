# SHforHLSL
Single-header SH support library for HLSL 2021. It implements types and utility functions for working with low-order spherical harmonics, focused on use cases for graphics.

## How to Use
Just include the header directly from HLSL 2021+ code.

## Features
Currently this library has support for L1 (2 bands, 4 coefficients) and L2 (3 bands, 9 coefficients) SH. Depending on the author and material you're reading, you may see L1 referred to as both first-order or second-order, and L2 referred to as second-order or third-order. Ravi Ramamoorthi tends to refer to three bands as second-order, and Peter-Pike Sloan tends to refer to three bands as third-order. This library always uses L1 and L2 for clarity.

The L1 and L2 core types can be templated on floating-point scalar and vector types, and are intended to be used with float, float3, half, and half3 (with `-enable-16bit-types` passed during compilation). When fp16 types are used, the helper functions take fp16 arguments to try to void implicit conversions where possible.

The core L1 and L2 structs support basic operator overloading for summing/subtracting two sets of SH coefficients as well as multiplying/dividing the set of SH coefficients by a value. The following utility functions are also available:

* L2toL1
* Lerp
* ProjectOntoL1
* ProjectOntoL2
* DotProduct
* Evaluate
* ConvolveWithZH
* ConvolveWithCosineLobe
* OptimalLinearDirection
* ApproximateDirectionalLight
* CalculateIrradiance
* CalculateIrradianceGeomerics
* CalculateIrradianceL1ZH3Hallucinate
* Rotate

## Examples

Example #1: integrating and projecting radiance onto L2 SH

```cpp
SH::L2 radianceSH = SH::L2::Zero();
for(uint sampleIndex = 0; sampleIndex < NumSamples; ++sampleIndex)
{
    float2 u1u2 = RandomFloat2(sampleIndex, NumSamples);
    float3 sampleDirection = SampleDirectionSphere(u1u2);
    float3 sampleRadiance = CalculateIncomingRadiance(sampleDirection);
    radianceSH += SH::ProjectOntoL2(sampleDirection, sampleRadiance);
}
radianceSH *= 1.0f / (NumSamples * SampleDirectionSphere_PDF());
```

Example #2: calculating diffuse lighting for a surface from radiance projected onto L2 SH

```cpp
SH::L2 radianceSH = FetchRadianceSH(surfacePosition);
float3 diffuseLighting = SH::CalculateIrradiance(radianceSH, surfaceNormal) * (diffuseAlbedo / Pi);
```

## License

SH.hlsli is licensed under the MIT license
