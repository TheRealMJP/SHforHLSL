//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "../PCH.h"

#include "../InterfacePointers.h"
#include "../Utility.h"
#include "../Containers.h"
#include "DX12.h"
#include "DX12_Upload.h"
#include "DX12_Helpers.h"
#include "../Shaders/ShaderShared.h"

namespace SampleFramework12
{

struct PersistentDescriptorAlloc
{
    D3D12_CPU_DESCRIPTOR_HANDLE Handles[DX12::RenderLatency] = { };
    DescriptorIndex Index = InvalidDescriptorIndex;
};

struct TempDescriptorAlloc
{
    D3D12_CPU_DESCRIPTOR_HANDLE StartCPUHandle = { };
    D3D12_GPU_DESCRIPTOR_HANDLE StartGPUHandle = { };
    DescriptorIndex StartIndex = InvalidDescriptorIndex;
};

// Wrapper for D3D12 descriptor heaps that supports persistent and temporary allocations
struct DescriptorHeap
{
    ID3D12DescriptorHeap* Heaps[DX12::RenderLatency] = { };
    uint32 NumPersistent = 0;
    uint32 PersistentAllocated = 0;
    Array<DescriptorIndex> DeadList;
    uint32 NumTemporary = 0;
    volatile int64 TemporaryAllocated = 0;
    uint32 HeapIndex = 0;
    uint32 NumHeaps = 0;
    uint32 DescriptorSize = 0;
    bool32 ShaderVisible = false;
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    D3D12_CPU_DESCRIPTOR_HANDLE CPUStart[DX12::RenderLatency] = { };
    D3D12_GPU_DESCRIPTOR_HANDLE GPUStart[DX12::RenderLatency] = { };
    SRWLOCK Lock = SRWLOCK_INIT;

    void Init(uint32 numPersistent, uint32 numTemporary, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible);
    void Shutdown();

    PersistentDescriptorAlloc AllocatePersistent(DescriptorIndex index = InvalidDescriptorIndex);
    void FreePersistent(DescriptorIndex& idx);
    void FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle);
    void FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle);

    TempDescriptorAlloc AllocateTemporary(uint32 count);
    void EndFrame();

    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandleFromIndex(uint32 descriptorIdx) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUHandleFromIndex(uint32 descriptorIdx) const;

    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandleFromIndex(uint32 descriptorIdx, uint64 heapIdx) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUHandleFromIndex(uint32 descriptorIdx, uint64 heapIdx) const;

    DescriptorIndex IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;
    DescriptorIndex IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) const;

    ID3D12DescriptorHeap* CurrentHeap() const;
    uint32 TotalNumDescriptors() const { return NumPersistent + NumTemporary; }
};

struct BufferInit
{
    uint64 Size = 0;
    uint64 Alignment = 0;
    bool32 Dynamic = false;
    bool32 CPUAccessible = false;
    bool32 AllowUAV = false;
    bool32 RTAccelStructure = false;
    const void* InitData = nullptr;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = uint64(-1);
    const wchar* Name = nullptr;
};

struct BufferReadToWriteBarrierDesc
{
    D3D12_BARRIER_SYNC SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_ACCESS AccessBefore = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    D3D12_BARRIER_ACCESS AccessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
};

struct BufferWriteToReadBarrierDesc
{
    D3D12_BARRIER_SYNC SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_ACCESS AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    D3D12_BARRIER_ACCESS AccessAfter = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
};

struct BufferWriteToWriteBarrierDesc
{
    D3D12_BARRIER_SYNC SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_ACCESS AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    D3D12_BARRIER_ACCESS AccessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
};

struct Buffer
{
    ID3D12Resource* Resource = nullptr;
    uint64 CurrBuffer = 0;
    uint8* CPUAddress = 0;
    uint64 GPUAddress = 0;
    uint64 Alignment = 0;
    uint64 Size = 0;
    bool32 Dynamic = false;
    bool32 CPUAccessible = false;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = 0;
    uint64 UploadFrame = uint64(-1);
    uint64 CreateFrame = uint64(-1);

    void Initialize(const BufferInit& init);
    void Shutdown();

    MapResult Map();
    MapResult MapAndSetData(const void* data, uint64 dataSize);
    template<typename T> MapResult MapAndSetData(const T& data) { return MapAndSetData(&data, sizeof(T)); }
    uint64 QueueUpload(ID3D12Resource* srcResource, uint64 srcOffset, uint64 srcSize, uint64 dstOffset);

    D3D12_BUFFER_BARRIER ReadToWriteBarrier(BufferReadToWriteBarrierDesc desc = BufferReadToWriteBarrierDesc()) const;
    D3D12_BUFFER_BARRIER WriteToReadBarrier(BufferWriteToReadBarrierDesc desc = BufferWriteToReadBarrierDesc()) const;
    D3D12_BUFFER_BARRIER WriteToWriteBarrier(BufferWriteToWriteBarrierDesc desc = BufferWriteToWriteBarrierDesc()) const;

    uint64 CycleBuffer();

    bool Initialized() const { return Size > 0; }

    #if UseAsserts_
        bool ReadyForBinding() const;
    #endif
};

// For aligning to float4 boundaries
#define Float4Align __declspec(align(16))
#define Float4Align_ __declspec(align(16))

struct ConstantBufferInit
{
    uint64 Size = 0;
    bool32 Dynamic = true;
    bool32 CPUAccessible = true;
    const void* InitData = nullptr;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = 0;
    const wchar* Name = nullptr;
};

struct ConstantBuffer
{
    Buffer InternalBuffer;
    uint64 CurrentGPUAddress = 0;

    void Initialize(const ConstantBufferInit& init);
    void Shutdown();

    void SetAsGfxRootParameter(ID3D12GraphicsCommandList* cmdList, uint32 rootParameter) const;
    void SetAsComputeRootParameter(ID3D12GraphicsCommandList* cmdList, uint32 rootParameter) const;

    void* Map();
    template<typename T> T* Map() { return reinterpret_cast<T*>(Map()); }
    void MapAndSetData(const void* data, uint64 dataSize);
    template<typename T> void MapAndSetData(const T& data) { MapAndSetData(&data, sizeof(T)); }
    void QueueUpload(ID3D12Resource* srcResource, uint64 srcOffset, uint64 srcSize, uint64 dstOffset);
};

struct StructuredBufferInit
{
    uint64 Stride = 0;
    uint64 NumElements = 0;
    bool32 CreateUAV = false;
    bool32 Dynamic = false;
    bool32 CPUAccessible = false;
    const void* InitData = nullptr;
    bool32 ShaderTable = false;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = 0;
    const wchar* Name = nullptr;
};

struct StructuredBuffer
{
    Buffer InternalBuffer;
    uint64 Stride = 0;
    uint64 NumElements = 0;
    DescriptorIndex SRV = InvalidDescriptorIndex;
    bool32 IsShaderTable = false;
    DescriptorIndex UAV = InvalidDescriptorIndex;
    uint64 GPUAddress = 0;

    void Initialize(const StructuredBufferInit& init);
    void Shutdown();

    D3D12_VERTEX_BUFFER_VIEW VBView() const;
    D3D12_INDEX_BUFFER_VIEW IBView() const;
    ID3D12Resource* Resource() const { return InternalBuffer.Resource; }

    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE ShaderTable(uint64 startElement = 0, uint64 numElements = uint64(-1)) const;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE ShaderRecord(uint64 element) const;

    void* Map();
    template<typename T> T* Map() { return reinterpret_cast<T*>(Map()); }
    void MapAndSetData(const void* data, uint64 numElements);
    void QueueUpload(ID3D12Resource* srcResource, uint64 srcOffset, uint64 srcNumElements, uint64 dstElemOffset);

    uint64 CycleBuffer();

private:

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc(uint64 bufferIdx) const;
    void UpdateDynamicSRV() const;
};

struct FormattedBufferInit
{
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    uint64 NumElements = 0;
    bool32 CreateUAV = false;
    bool32 Dynamic = false;
    bool32 CPUAccessible = false;
    const void* InitData = nullptr;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = 0;
    const wchar* Name = nullptr;
};

struct FormattedBuffer
{
    Buffer InternalBuffer;
    uint64 Stride = 0;
    uint64 NumElements = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    DescriptorIndex SRV = InvalidDescriptorIndex;
    DescriptorIndex UAV = InvalidDescriptorIndex;
    uint64 GPUAddress = 0;

    void Initialize(const FormattedBufferInit& init);
    void Shutdown();

    D3D12_INDEX_BUFFER_VIEW IBView() const;
    ID3D12Resource* Resource() const { return InternalBuffer.Resource; }

    void* Map();
    template<typename T> T* Map() { return reinterpret_cast<T*>(Map()); };
    void MapAndSetData(const void* data, uint64 numElements);
    void QueueUpload(ID3D12Resource* srcResource, uint64 srcOffset, uint64 srcNumElements, uint64 dstElemOffset);

    uint64 CycleBuffer();

private:

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc(uint64 bufferIdx) const;
    void UpdateDynamicSRV() const;
};

struct RawBufferInit
{
    uint64 NumElements = 0;
    bool32 CreateUAV = false;
    bool32 Dynamic = false;
    bool32 CPUAccessible = false;
    const void* InitData = nullptr;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = 0;
    const wchar* Name = nullptr;
};

struct RawBuffer
{
    Buffer InternalBuffer;
    uint64 NumElements = 0;
    DescriptorIndex SRV = InvalidDescriptorIndex;
    DescriptorIndex UAV = InvalidDescriptorIndex;
    uint64 GPUAddress = 0;

    static const uint64 Stride = 4;

    void Initialize(const RawBufferInit& init);
    void Shutdown();

    ID3D12Resource* Resource() const { return InternalBuffer.Resource; }

    void* Map();
    template<typename T> T* Map() { return reinterpret_cast<T*>(Map()); };
    void MapAndSetData(const void* data, uint64 numElements);
    void QueueUpload(ID3D12Resource* srcResource, uint64 srcOffset, uint64 srcNumElements, uint64 dstElemOffset);

    uint64 CycleBuffer();

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc(uint64 bufferIdx) const;

private:

    void UpdateDynamicSRV() const;
};

struct RTAccelStructureInit
{
    uint64 Size = 0;
    ID3D12Heap* Heap = nullptr;
    uint64 HeapOffset = 0;
    const wchar* Name = nullptr;
};

struct RTAccelStructure
{
    Buffer InternalBuffer;
    uint64 Size = 0;
    DescriptorIndex SRV = InvalidDescriptorIndex;
    uint64 GPUAddress = 0;

    void Initialize(const RTAccelStructureInit& init);
    void Shutdown();

    ID3D12Resource* Resource() const { return InternalBuffer.Resource; }

    D3D12_BUFFER_BARRIER TopLevelPostBuildBarrier() const;
    D3D12_BUFFER_BARRIER BottomLevelPostBuildBarrier() const;
};

struct ReadbackBuffer
{
    ID3D12Resource* Resource = nullptr;
    uint64 Size = 0;

    void Initialize(uint64 size);
    void Shutdown();

    const void* Map() const;
    template<typename T> const T* Map() const { return reinterpret_cast<const T*>(Map()); };
    void Unmap() const;
};

struct Fence
{
    ID3D12Fence* D3DFence = nullptr;
    HANDLE FenceEvent = INVALID_HANDLE_VALUE;

    void Init(uint64 initialValue = 0);
    void Shutdown();

    void Signal(ID3D12CommandQueue* queue, uint64 fenceValue);
    void Wait(uint64 fenceValue);
    bool Signaled(uint64 fenceValue);
    void Clear(uint64 fenceValue);
};

struct Texture
{
    DescriptorIndex SRV;
    ID3D12Resource* Resource = nullptr;
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 0;
    uint32 NumMips = 0;
    uint32 ArraySize = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    bool32 Cubemap = false;

    bool Valid() const
    {
        return Resource != nullptr;
    }

    void Shutdown();

    D3D12_BARRIER_SUBRESOURCE_RANGE BarrierRange(uint32 startMipLevel, uint32 numMipLevels, uint32 startArraySlice, uint32 numArraySlices) const;
};

struct RenderTextureInit
{
    uint32 Width = 0;
    uint32 Height = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    uint32 MSAASamples = 1;
    uint32 ArraySize = 1;
    bool32 CreateUAV = false;
    bool32 CreateRTV = true;
    bool32 CubeMap = false;
    uint32 NumMips = 1;
    D3D12_BARRIER_LAYOUT InitialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
    const wchar* Name = nullptr;
};

enum QueueVisibility : uint32
{
    Direct = 0,
    Compute,
    ComputeAndDirect,

    NumQueueVisibilities,
};

struct RTWritableBarrierDesc
{
    bool FirstAccess = false;
    bool Discard = false;
    QueueVisibility QueueVisibilityAfter = Direct;
    D3D12_BARRIER_SYNC SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_ACCESS AccessBefore = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    D3D12_BARRIER_LAYOUT LayoutBefore = D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE;
    uint32 StartMipLevel = 0;
    uint32 NumMipLevels = uint32(-1);
    uint32 StartArraySlice = 0;
    uint32 NumArraySlices = uint32(-1);
};

struct RTReadableBarrierDesc
{
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
    QueueVisibility QueueVisibilityBefore = Direct;
    QueueVisibility QueueVisibilityAfter = Direct;
    uint32 StartMipLevel = 0;
    uint32 NumMipLevels = uint32(-1);
    uint32 StartArraySlice = 0;
    uint32 NumArraySlices = uint32(-1);
};

struct RTMemoryBarrierDesc
{
    D3D12_BARRIER_SYNC SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_LAYOUT Layout = D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS;
    uint32 StartMipLevel = 0;
    uint32 NumMipLevels = uint32(-1);
    uint32 StartArraySlice = 0;
    uint32 NumArraySlices = uint32(-1);
};

struct RenderTexture
{
    Texture Texture;
    D3D12_CPU_DESCRIPTOR_HANDLE RTV = { };
    DescriptorIndex UAV;
    Array<D3D12_CPU_DESCRIPTOR_HANDLE> ArrayRTVs;
    Array<DescriptorIndex> MipLevelUAVs;
    uint32 MSAASamples = 0;
    uint32 MSAAQuality = 0;
    bool32 HasRTV = false;

    void Initialize(const RenderTextureInit& init);
    void Shutdown();

    D3D12_TEXTURE_BARRIER RTWritableBarrier(RTWritableBarrierDesc desc = RTWritableBarrierDesc()) const;
    D3D12_TEXTURE_BARRIER UAVWritableBarrier(RTWritableBarrierDesc desc = RTWritableBarrierDesc()) const;
    D3D12_TEXTURE_BARRIER RTToShaderReadableBarrier(RTReadableBarrierDesc desc = RTReadableBarrierDesc()) const;
    D3D12_TEXTURE_BARRIER UAVToShaderReadableBarrier(RTReadableBarrierDesc desc = RTReadableBarrierDesc()) const;
    D3D12_TEXTURE_BARRIER MemoryBarrier(RTMemoryBarrierDesc desc = RTMemoryBarrierDesc()) const;

    uint32 SRV() const { return Texture.SRV; }
    uint32 Width() const { return Texture.Width; }
    uint32 Height() const { return Texture.Height; }
    DXGI_FORMAT Format() const { return Texture.Format; }
    ID3D12Resource* Resource() const { return Texture.Resource; }
    uint32 SubResourceIndex(uint32 mipLevel, uint32 arraySlice) const { return arraySlice * Texture.NumMips + mipLevel; }
};

struct VolumeTextureInit
{
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    D3D12_BARRIER_LAYOUT InitialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
    const wchar* Name = nullptr;
};

struct VolumeTexture
{
    Texture Texture;
    DescriptorIndex UAV;

    void Initialize(const VolumeTextureInit& init);
    void Shutdown();

    uint32 SRV() const { return Texture.SRV; }
    uint32 Width() const { return Texture.Width; }
    uint32 Height() const { return Texture.Height; }
    uint32 Depth() const { return Texture.Depth; }
    DXGI_FORMAT Format() const { return Texture.Format; }
    ID3D12Resource* Resource() const { return Texture.Resource; }
};

struct DepthBufferInit
{
    uint32 Width = 0;
    uint32 Height = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    uint32 MSAASamples = 1;
    uint32 ArraySize = 1;
    D3D12_BARRIER_LAYOUT InitialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
    const wchar* Name = nullptr;
};

struct DepthWritableBarrierDesc
{
    bool FirstAccess = false;
    bool Discard = false;
    D3D12_BARRIER_SYNC SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_ACCESS AccessBefore = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    D3D12_BARRIER_LAYOUT LayoutBefore = D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE;
    uint32 StartArraySlice = 0;
    uint32 NumArraySlices = uint32(-1);
};

struct DepthReadableBarrierDesc
{
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING | D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    D3D12_BARRIER_ACCESS AccessAfter = D3D12_BARRIER_ACCESS_SHADER_RESOURCE | D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    D3D12_BARRIER_LAYOUT LayoutAfter = D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ;
    uint32 StartArraySlice = 0;
    uint32 NumArraySlices = uint32(-1);
};

struct DepthShaderReadableBarrierDesc
{
    D3D12_BARRIER_SYNC SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
    D3D12_BARRIER_ACCESS AccessAfter = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    D3D12_BARRIER_LAYOUT LayoutAfter = D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE;
    uint32 StartArraySlice = 0;
    uint32 NumArraySlices = uint32(-1);
};

struct DepthBuffer
{
    Texture Texture;
    D3D12_CPU_DESCRIPTOR_HANDLE DSV = { };
    D3D12_CPU_DESCRIPTOR_HANDLE ReadOnlyDSV = { };
    Array<D3D12_CPU_DESCRIPTOR_HANDLE> ArrayDSVs;
    uint32 MSAASamples = 0;
    uint32 MSAAQuality = 0;
    DXGI_FORMAT DSVFormat = DXGI_FORMAT_UNKNOWN;

    void Initialize(const DepthBufferInit& init);
    void Shutdown();

    D3D12_TEXTURE_BARRIER DepthWritableBarrier(DepthWritableBarrierDesc desc = DepthWritableBarrierDesc()) const;
    D3D12_TEXTURE_BARRIER DepthReadableBarrier(DepthReadableBarrierDesc desc = DepthReadableBarrierDesc()) const;
    D3D12_TEXTURE_BARRIER ShaderReadableBarrier(DepthShaderReadableBarrierDesc desc = DepthShaderReadableBarrierDesc()) const;

    uint32 SRV() const { return Texture.SRV; }
    uint32 Width() const { return Texture.Width; }
    uint32 Height() const { return Texture.Height; }
    ID3D12Resource* Resource() const { return Texture.Resource; }
};

struct FeedbackTextureInit
{
    const Texture* PairedTexture = nullptr;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    D3D12_BARRIER_LAYOUT InitialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
    D3D12_MIP_REGION MipRegion = { };
    const wchar* Name = nullptr;
};

struct FeedbackTexture
{
    Texture Texture;
    D3D12_MIP_REGION MipRegion = { };
    DescriptorIndex UAV;

    void Initialize(const FeedbackTextureInit& init);
    void Shutdown();

    uint32 Width() const { return Texture.Width; }
    uint32 Height() const { return Texture.Height; }
    DXGI_FORMAT Format() const { return Texture.Format; }
    ID3D12Resource* Resource() const { return Texture.Resource; }
    uint32 DecodeWidth() const;
    uint32 DecodeHeight() const;
    uint32 DecodeBufferSize() const;
};

struct PIXMarker
{
    ID3D12GraphicsCommandList* CmdList = nullptr;

    PIXMarker(ID3D12GraphicsCommandList* cmdList, const wchar* msg) : CmdList(cmdList)
    {
        PIXBeginEvent(cmdList, 0, msg);
    }

    PIXMarker(ID3D12GraphicsCommandList* cmdList, const char* msg) : CmdList(cmdList)
    {
        PIXBeginEvent(cmdList, 0, msg);
    }

    ~PIXMarker()
    {
        PIXEndEvent(CmdList);
    }
};

}