# SHforHLSL
SH.hlsli is a single-header spherical harmonics support library for HLSL 2021. It implements types and utility functions for working with low-order SH, focused on use cases for graphics.

An alternative "Lite" version is also available in SH_Lite.hlsl, which compiles is compatible with older FXC-style HLSL.

## How to Use
Just include the SH.hlsli header directly from HLSL 2021+ code. Or for pre-HLSL 2021, instead include SH_Lite.hlsli.

## Features
Currently this library has support for L1 (2 bands, 4 coefficients) and L2 (3 bands, 9 coefficients) SH. Depending on the author and material you're reading, you may see L1 referred to as both first-order or second-order, and L2 referred to as second-order or third-order. Ravi Ramamoorthi tends to refer to three bands as second-order, and Peter-Pike Sloan tends to refer to three bands as third-order. This library always uses L1 and L2 for clarity.

The core `SH` type as well as the `L1` and `L2` aliases are templated on the primitive scalar type (`T`) as well as the number of vector components (`N`). They are intended to be used with 1 or 3 components paired with the `float` and `half` primitive types (with `-enable-16bit-types` passed during compilation). When fp16 types are used, the helper functions take fp16 arguments to try to avoid implicit conversions where possible.

The core `SH` type supports basic operator overloading for summing/subtracting two sets of SH coefficients as well as multiplying/dividing the set of SH coefficients by a value. The following utility functions are also available:

* L2toL1
* ToRGB
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

## "Lite" Version

SH_Lite.hlsli is a template-less version of SH.hlsli that is compatible with pre-HLSL 2021. You can use this if you're still stuck with FXC (I'm sorry), or if you would prefer to avoid all of the template bloat. The interface and functions are mostly identical, with the following limitations:

* No operator overloads. Instead `Add`, `Subtract`, `Multiply`, and `Divide` functions are provided.
* No support for fp16, either through the newer explicit `float16_t` types or the older `min16float` flexible precision types.

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

There is a simple compute shader (`CompileTest.hlsl`) intended for testing that all of the functions compile successfully for all valid template types. Running `CompileTest.bat` will invoke compilation. dxc.exe + dxcompiler.dll + dxil.dll can be dropped into the same directory as the batch file to use a specific version of the compiler. `CompileTest_Lite.hlsl` is also compiled with both DXC and FXC, and tests the Lite version of the header.

For more thorough visual inspection of the results, the SHTest subfolder contains a full DX12 project that renders a sphere using SH irradiance and rotation. This project tests all of the major SH types (L1, L1_RGB, L2_F16, etc.) for both the original SH.hlsli as well as the the more limited SH_Lite.hlsli. For the L1 modes, both the Geomerics as well as the ZH3Hallucinate methods for calculating irradiance are used by splitting the sphere in half along the Y axis.

![image](https://github.com/user-attachments/assets/1f43a796-d66a-4862-bd21-6545c2b9b190)

## License

SH.hlsli, SH_Lite.hlsli, and SHTest.hlsli are licensed under the MIT license
