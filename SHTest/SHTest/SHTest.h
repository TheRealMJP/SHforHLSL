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

#include <PCH.h>

#include <App.h>
#include <Graphics/GraphicsTypes.h>
#include "AppSettings.h"

using namespace SampleFramework12;

class SHTest : public App
{

protected:

    CompiledShaderPtr testVS;
    CompiledShaderPtr testPS;
    CompiledShaderPtr testPSLite;
    ID3D12PipelineState* testPSO = nullptr;
    ID3D12PipelineState* testPSOLite = nullptr;

    virtual void Initialize() override;
    virtual void Shutdown() override;

    virtual void Render(const Timer& timer) override;
    virtual void Update(const Timer& timer) override;

    virtual void BeforeReset() override;
    virtual void AfterReset() override;

    virtual void CreatePSOs() override;
    virtual void DestroyPSOs() override;

public:

    SHTest(const wchar* cmdLine);
};
