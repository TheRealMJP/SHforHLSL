#pragma once

#include <PCH.h>
#include <Settings.h>
#include <Graphics\GraphicsTypes.h>

using namespace SampleFramework12;

namespace AppSettings
{

    extern BoolSetting EnableVSync;

    struct AppSettingsCBuffer
    {
        uint32 Dummy;
    };

    extern ConstantBuffer CBuffer;
    const extern uint32 CBufferRegister;

    void Initialize();
    void Shutdown();
    void Update(uint32 displayWidth, uint32 displayHeight, const Float4x4& viewMatrix);
    void UpdateCBuffer();
    void BindCBufferGfx(ID3D12GraphicsCommandList* cmdList, uint32 rootParameter);
    void BindCBufferCompute(ID3D12GraphicsCommandList* cmdList, uint32 rootParameter);
    void GetShaderCompileOptions(CompileOptions& opts);
    bool ShaderCompileOptionsChanged();
};

// ================================================================================================

namespace AppSettings
{
}