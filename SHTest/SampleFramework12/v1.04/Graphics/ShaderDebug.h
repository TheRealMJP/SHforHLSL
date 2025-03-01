//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "..\\PCH.h"

namespace SampleFramework12
{

namespace ShaderDebug
{
    void Initialize();
    void Shutdown();

    // void CreatePSOs(DXGI_FORMAT rtFormat, DXGI_FORMAT dsvFormat);
    // void DestroyPSOs();

    void BeginRender(ID3D12GraphicsCommandList7* cmdList, uint32 cursorX, uint32 cursorY);
    void EndRender(ID3D12GraphicsCommandList7* cmdList);
}

}