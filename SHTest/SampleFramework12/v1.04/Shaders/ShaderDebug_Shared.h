//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "ShaderShared.h"

namespace ShaderDebug
{

struct DebugInfo
{
    DescriptorIndex PrintBuffer;
    ShaderUint PrintBufferSize;
    ShaderUint2 CursorXY;
};

SharedConstant_ DescriptorIndex MagicDebugBufferIndex = 1024;
SharedConstant_ uint32_t MaxDebugPrintArgs = 16;

enum ArgCode
{
    DebugPrint_Uint = 0,
    DebugPrint_Uint2,
    DebugPrint_Uint3,
    DebugPrint_Uint4,
    DebugPrint_Int,
    DebugPrint_Int2,
    DebugPrint_Int3,
    DebugPrint_Int4,
    DebugPrint_Float,
    DebugPrint_Float2,
    DebugPrint_Float3,
    DebugPrint_Float4,

    NumDebugPrintArgCodes,
};

struct DebugPrintHeader
{
    ShaderUint NumBytes;
    ShaderUint StringSize;
    ShaderUint NumArgs;
};

} // namespace ShaderDebug