//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "PCH.h"

#include "ShaderDebug.h"
#include "GraphicsTypes.h"
#include "DX12_Helpers.h"
#include "../App.h"
#include "../Shaders/ShaderDebug_Shared.h"

using namespace ShaderDebug;

namespace SampleFramework12
{

namespace ShaderDebug
{

static RawBuffer DebugInfoBuffer;
static RawBuffer PrintBuffer;
static ReadbackBuffer PrintReadbackBuffers[DX12::RenderLatency];

static const std::string ArgPlaceHolders[] =
{
    "{0}",
    "{1}",
    "{2}",
    "{3}",
    "{4}",
    "{5}",
    "{6}",
    "{7}",
    "{8}",
    "{9}",
    "{10}",
    "{11}",
    "{12}",
    "{13}",
    "{14}",
    "{15}",
};
StaticAssert_(ArraySize_(ArgPlaceHolders) == MaxDebugPrintArgs);

static const uint32 ArgCodeSizes[] =
{
    sizeof(uint32),     // DebugPrint_Uint
    sizeof(Uint2),      // DebugPrint_Uint2
    sizeof(Uint3),      // DebugPrint_Uint3
    sizeof(Uint4),      // DebugPrint_Uint4
    sizeof(int32),      // DebugPrint_Int
    sizeof(Int2),       // DebugPrint_Int2
    sizeof(Int3),       // DebugPrint_Int3
    sizeof(Int4),       // DebugPrint_Int4
    sizeof(float),      // DebugPrint_Float
    sizeof(Float2),     // DebugPrint_Float2
    sizeof(Float3),     // DebugPrint_Float3
    sizeof(Float4),     // DebugPrint_Float4
};

static void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos)
    {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

void Initialize()
{
    DebugInfoBuffer.Initialize({
        .NumElements = sizeof(DebugInfo) / 4,
        .Dynamic = true,
        .CPUAccessible = true,
        .Name = L"Debug Info Buffer",
    });

    const PersistentDescriptorAlloc alloc = DX12::SRVDescriptorHeap.AllocatePersistent(MagicDebugBufferIndex);
    DX12::SRVDescriptorHeap.FreePersistent(DebugInfoBuffer.SRV);
    DebugInfoBuffer.SRV = alloc.Index;

    for(uint32 i = 0; i < ArraySize_(alloc.Handles); ++i)
    {
        const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = DebugInfoBuffer.SRVDesc(i);
        DX12::Device->CreateShaderResourceView(DebugInfoBuffer.Resource(), &srvDesc, alloc.Handles[i]);
    }

    PrintBuffer.Initialize({
        .NumElements = 1024 * 1024  * 4,
        .CreateUAV = true,
        .Name = L"Shader Debug Print Buffer",
    });

    for(ReadbackBuffer& buffer : PrintReadbackBuffers)
        buffer.Initialize(PrintBuffer.InternalBuffer.Size);
}

void Shutdown()
{
    DebugInfoBuffer.Shutdown();
    PrintBuffer.Shutdown();
    for(ReadbackBuffer& buffer : PrintReadbackBuffers)
        buffer.Shutdown();
}

/*void CreatePSOs(DXGI_FORMAT rtFormat, DXGI_FORMAT dsvFormat)
{
}

void DestroyPSOs()
{
}*/

struct DebugPrintReader
{
    const uint8* PrintBufferData = nullptr;
    uint32 BufferSize = 0;
    uint32 TotalNumBytes = 0;
    uint32 CurrOffset = 0;

    DebugPrintReader(const uint8* printBufferData, uint32 bufferSize)
    {
        PrintBufferData = printBufferData;
        BufferSize = bufferSize;
        memcpy(&TotalNumBytes, printBufferData, sizeof(uint32));
        TotalNumBytes += sizeof(uint32);
        TotalNumBytes = Min(TotalNumBytes, BufferSize);
        CurrOffset = sizeof(uint32);
    }

    template<typename T> T Consume(T defVal = T())
    {
        T x;

        if(CurrOffset + sizeof(T) > TotalNumBytes)
            return defVal;

        memcpy(&x, PrintBufferData + CurrOffset, sizeof(T));
        CurrOffset += sizeof(T);
        return x;
    }

    const char* ConsumeString(uint32 expectedStringSize)
    {
        if(expectedStringSize == 0)
            return "";

        if(CurrOffset + expectedStringSize > TotalNumBytes)
            return "";

        const char* stringData = reinterpret_cast<const char*>(PrintBufferData + CurrOffset);
        CurrOffset += expectedStringSize;
        if(stringData[expectedStringSize - 1] != '\0')
            return "";

        return stringData;
    }

    bool HasMoreData(uint32 numBytes) const
    {
        return (CurrOffset + numBytes) <= TotalNumBytes;
    }
};

static std::string MakeArgString(DebugPrintReader& printReader, ArgCode argCode)
{
    if(argCode == DebugPrint_Uint)
    {
        return MakeString("%u", printReader.Consume<uint32>());
    }
    if(argCode == DebugPrint_Uint2)
    {
        const Uint2 x = printReader.Consume<Uint2>();
        return MakeString("(%u, %u)", x.x, x.y);
    }
    if(argCode == DebugPrint_Uint3)
    {
        const Uint3 x = printReader.Consume<Uint3>();
        return MakeString("(%u, %u, %u)", x.x, x.y, x.z);
    }
    if(argCode == DebugPrint_Uint4)
    {
        const Uint4 x = printReader.Consume<Uint4>();
        return MakeString("(%u, %u, %u, %u)", x.x, x.y, x.z, x.w);
    }
    if(argCode == DebugPrint_Int)
    {
        return MakeString("%i", printReader.Consume<int32>());
    }
    if(argCode == DebugPrint_Int2)
    {
        const Int2 x = printReader.Consume<Int2>();
        return MakeString("(%i, %i)", x.x, x.y);
    }
    if(argCode == DebugPrint_Int3)
    {
        const Int3 x = printReader.Consume<Int3>();
        return MakeString("(%i, %i, %i)", x.x, x.y, x.z);
    }
    if(argCode == DebugPrint_Int4)
    {
        const Int4 x = printReader.Consume<Int4>();
        return MakeString("(%i, %i, %i, %i)", x.x, x.y, x.z, x.w);
    }
    if(argCode == DebugPrint_Float)
    {
        return MakeString("%f", printReader.Consume<float>());
    }
    if(argCode == DebugPrint_Float2)
    {
        const Float2 x = printReader.Consume<Float2>();
        return MakeString("(%f, %f)", x.x, x.y);
    }
    if(argCode == DebugPrint_Float3)
    {
        const Float3 x = printReader.Consume<Float3>();
        return MakeString("(%f, %f, %f)", x.x, x.y, x.z);
    }
    if(argCode == DebugPrint_Float4)
    {
        const Float4 x = printReader.Consume<Float4>();
        return MakeString("(%f, %f, %f, %f)", x.x, x.y, x.z, x.w);
    }

    Assert_(false);
    return "???";
}

void BeginRender(ID3D12GraphicsCommandList7* cmdList, uint32 cursorX, uint32 cursorY)
{
    PIXMarker marker(cmdList, "ShaderDebug - BeginRender");

    DebugInfo debugInfo =
    {
        .PrintBuffer = PrintBuffer.SRV,
        .PrintBufferSize = uint32(PrintBuffer.InternalBuffer.Size),
        .CursorXY = { cursorX, cursorY },
    };
    DebugInfoBuffer.MapAndSetData(&debugInfo, sizeof(debugInfo) / 4);

    List<std::string> argStrings;

    if(DX12::CurrentCPUFrame >= DX12::RenderLatency)
    {
        const ReadbackBuffer& readbackBuffer = PrintReadbackBuffers[(DX12::CurrentCPUFrame + 1) % DX12::RenderLatency];
        DebugPrintReader printReader(readbackBuffer.Map<uint8>(), uint32(readbackBuffer.Size));
        while(printReader.HasMoreData(sizeof(DebugPrintHeader)))
        {
            const DebugPrintHeader header = printReader.Consume<DebugPrintHeader>(DebugPrintHeader{});
            if(header.NumBytes == 0 || printReader.HasMoreData(header.NumBytes) == false)
                break;

            std::string formatStr = printReader.ConsumeString(header.StringSize);
            if(formatStr.length() == 0)
                break;

            if(header.NumArgs > MaxDebugPrintArgs)
                break;

            argStrings.Reserve(header.NumArgs);
            for(uint32 argIdx = 0; argIdx < header.NumArgs; ++argIdx)
            {

                const ArgCode argCode = (ArgCode)printReader.Consume<uint8>(0xFF);
                if(argCode >= NumDebugPrintArgCodes)
                    break;

                const uint32 argSize = ArgCodeSizes[argCode];
                if(printReader.HasMoreData(argSize) == false)
                    break;

                const std::string argStr = MakeArgString(printReader, argCode);
                ReplaceStringInPlace(formatStr, ArgPlaceHolders[argIdx], argStr);
            }

            GlobalApp->AddToGPULog(formatStr.c_str());

            printReader.CurrOffset = AlignTo(printReader.CurrOffset, 4);
        }

        readbackBuffer.Unmap();
    }

    DX12::ClearRawBuffer(cmdList, PrintBuffer, Uint4(0, 0, 0, 0));

    DX12::Barrier(cmdList, PrintBuffer.InternalBuffer.WriteToWriteBarrier());
}

void EndRender(ID3D12GraphicsCommandList7* cmdList)
{
    PIXMarker marker(cmdList, "ShaderDebug - EndRender");

    DX12::Barrier(cmdList, PrintBuffer.InternalBuffer.WriteToReadBarrier( { .SyncAfter = D3D12_BARRIER_SYNC_COPY, .AccessAfter = D3D12_BARRIER_ACCESS_COPY_SOURCE } ));

    const ReadbackBuffer& readbackBuffer = PrintReadbackBuffers[DX12::CurrentCPUFrame % DX12::RenderLatency];
    cmdList->CopyResource(readbackBuffer.Resource, PrintBuffer.Resource());
}

}

}