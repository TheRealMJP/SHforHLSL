# SHforHLSL
Single-header spherical harmonics support library for HLSL 2021. It implements types and utility functions for working with low-order SH, focused on use cases for graphics.

## How to Use
Just include the header directly from HLSL 2021+ code.

## Features
Currently this library has support for L1 (2 bands, 4 coefficients) and L2 (3 bands, 9 coefficients) SH. Depending on the author and material you're reading, you may see L1 referred to as both first-order or second-order, and L2 referred to as second-order or third-order. Ravi Ramamoorthi tends to refer to three bands as second-order, and Peter-Pike Sloan tends to refer to three bands as third-order. This library always uses L1 and L2 for clarity.

The core `SH` type as well as the `L1` and `L2` aliases are templated on the primitive scalar type (`T`) as well as the number of vector components (`N`). They are intended to be used with 1 or 3 components paired with the `float` and `half` primitive types (with `-enable-16bit-types` passed during compilation). When fp16 types are used, the helper functions take fp16 arguments to try to avoid implicit conversions where possible.

The core `SH` type supports basic operator overloading for summing/subtracting two sets of SH coefficients as well as multiplying/dividing the set of SH coefficients by a value. The following utility functions are also available:

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
* ApproximateGGXAsL1ZH
* ApproximateGGXAsL2ZH
* ConvolveWithGGX
* ExtractSpecularDirLight
* Rotate

## Examples

Example #1: integrating and projecting radiance onto L2 SH

```cpp
SH::L2_RGB radianceSH = SH::L2_RGB::Zero();
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
SH::L2_RGB radianceSH = FetchRadianceSH(surfacePosition);
float3 diffuseLighting = SH::CalculateIrradiance(radianceSH, surfaceNormal) * (diffuseAlbedo / Pi);
```

## Testing

Currently there is just a simple compute shader (`CompileTest.hlsl`) intended for testing that all of the functions compile successfully for all valid template types. Running `CompileTest.bat` will invoke compilation. dxc.exe + dxcompiler.dll + dxil.dll can be dropped into the same directory as the batch file to use a specific version of the compiler.

## License

SH.hlsli is licensed under the MIT license
