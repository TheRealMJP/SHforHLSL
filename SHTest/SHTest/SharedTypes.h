//=================================================================================================
//
//  SHTest
//  by MJP
//  https://therealmjp.github.io/
//
//  All code and content licensed under the MIT license
//
//=================================================================================================

#pragma once

#if CPP_
    #include <Shaders/ShaderShared.h>
#else
    #include <ShaderShared.h>
#endif

enum TestModes : uint32_t
{
    TestMode_L1 = 0,
    TestMode_L1_RGB,
    TestMode_L2,
    TestMode_L2_RGB,

    TestMode_L1_FP16,
    TestMode_L1_RGB_FP16,
    TestMode_L2_FP16,
    TestMode_L2_RGB_FP16,
};

struct TestConstants
{
    ShaderFloat4x4 InvProjection;
    float Time;
    TestModes TestMode;
};