//=================================================================================================
//
//  D3D12 Memory Pool Performance Test
//  by MJP
//  https://therealmjp.github.io/
//
//  All code and content licensed under the MIT license
//
//=================================================================================================

#include <PCH.h>

#include <Window.h>
#include <Input.h>
#include <Utility.h>
#include <Graphics/SwapChain.h>
#include <Graphics/ShaderCompilation.h>
#include <Graphics/Profiler.h>
#include <Graphics/DX12.h>
#include <Graphics/DX12_Helpers.h>
#include <ImGui/ImGui.h>
#include <ImGuiHelper.h>

#include "SHTest.h"
#include "SharedTypes.h"
#include "AppSettings.h"

using namespace SampleFramework12;

SHTest::SHTest(const wchar* cmdLine) : App(L"SHforHLSL Test", cmdLine)
{
    swapChain.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
}

void SHTest::BeforeReset()
{
}

void SHTest::AfterReset()
{
}

void SHTest::Initialize()
{
    testVS = CompileFromFile((SampleFrameworkDir() + L"Shaders\\FullScreenTriangle.hlsl").c_str(), "FullScreenTriangleVS", ShaderType::Vertex);

    CompileOptions opts;
    opts.Add("UseLite_", 0);
    testPS = CompileFromFile(L"SHTest.hlsl", "SHTestPS", ShaderType::Pixel, opts);

    opts.Reset();
    opts.Add("UseLite_", 1);
    testPSLite = CompileFromFile(L"SHTest.hlsl", "SHTestPS", ShaderType::Pixel, opts);
}

void SHTest::Shutdown()
{

}

void SHTest::CreatePSOs()
{
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = DX12::UniversalRootSignature;
        psoDesc.VS = testVS.ByteCode();
        psoDesc.PS = testPS.ByteCode();
        psoDesc.RasterizerState = DX12::GetRasterizerState(RasterizerState::NoCull);
        psoDesc.BlendState = DX12::GetBlendState(BlendState::Disabled);
        psoDesc.DepthStencilState = DX12::GetDepthState(DepthState::Disabled);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = swapChain.Format();
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        DXCall(DX12::Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&testPSO)));

        psoDesc.PS = testPSLite.ByteCode();
        DXCall(DX12::Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&testPSOLite)));
    }
}

void SHTest::DestroyPSOs()
{
    DX12::DeferredRelease(testPSO);
    DX12::DeferredRelease(testPSOLite);
}

void SHTest::Update(const Timer& timer)
{
    CPUProfileBlock cpuProfileBlock("Update");

    // Toggle VSYNC
    swapChain.SetVSYNCEnabled(AppSettings::EnableVSync ? true : false);

}

struct Test
{
    ID3D12PipelineState* PSO = nullptr;
    TestModes TestMode = (TestModes)0;
    const wchar* Name = L"";
};

void SHTest::Render(const Timer& timer)
{
    ID3D12GraphicsCommandList10* cmdList = DX12::CmdList;

    CPUProfileBlock cpuProfileBlock("Render");
    ProfileBlock gpuProfileBlock(cmdList, "Render Total");

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1] = { swapChain.BackBuffer().RTV };
    cmdList->OMSetRenderTargets(1, rtvHandles, false, nullptr);

    float clearColor[4] = { 0.2f, 0.4f, 0.8f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);

    DX12::SetViewport(cmdList, swapChain.Width(), swapChain.Height());

    const Test tests[]
    {
        {
            .PSO = testPSO,
            .TestMode = TestMode_L1,
            .Name = L"L1",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L1_RGB,
            .Name = L"L1_RGB",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L2,
            .Name = L"L2",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L2_RGB,
            .Name = L"L2_RGB",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L1_FP16,
            .Name = L"L1_FP16",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L1_RGB_FP16,
            .Name = L"L1_RGB_FP16",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L2_FP16,
            .Name = L"L2_FP16",
        },

        {
            .PSO = testPSO,
            .TestMode = TestMode_L2_RGB_FP16,
            .Name = L"L2_RGB_FP16",
        },

        {
            .PSO = testPSOLite,
            .TestMode = TestMode_L1,
            .Name = L"L1 (Lite)",
        },

        {
            .PSO = testPSOLite,
            .TestMode = TestMode_L1_RGB,
            .Name = L"L1_RGB (Lite)",
        },

        {
            .PSO = testPSOLite,
            .TestMode = TestMode_L2,
            .Name = L"L2 (Lite)",
        },

        {
            .PSO = testPSOLite,
            .TestMode = TestMode_L2_RGB,
            .Name = L"L2_RGB (Lite)",
        },

    };

    const uint32 numTests = ArraySize_(tests);
    const uint32 numRows = uint32(std::sqrt(float(numTests)));
    const uint32 numCols = (numTests + (numRows - 1)) / numRows;

    const uint32 vpWidth = uint32(float(swapChain.Width()) / numCols);
    const uint32 vpHeight = uint32(float(swapChain.Height()) / numRows);

    const Float4x4 proj = Float4x4(DirectX::XMMatrixPerspectiveFovLH(Pi_4, float(vpWidth) / vpHeight, 0.01f, 100.0f));
    const Float4x4 invProj = Float4x4::Invert(proj);
    const float time = timer.ElapsedSecondsF();

    for (uint32 testIndex = 0; testIndex < numTests; ++testIndex)
    {
        cmdList->SetPipelineState(tests[testIndex].PSO);
        cmdList->SetGraphicsRootSignature(DX12::UniversalRootSignature);

        AppSettings::BindCBufferGfx(cmdList, URS_AppSettings);

        TestConstants testConstants
        {
            .InvProjection = invProj,
            .Time = time,
            .TestMode = tests[testIndex].TestMode,
        };
        DX12::BindTempConstantBuffer(cmdList, testConstants, URS_ConstantBuffers + 0, CmdListMode::Graphics);

        const uint32 colIndex = testIndex % numCols;
        const uint32 rowIndex = testIndex / numCols;
        D3D12_VIEWPORT viewport =
        {
            .TopLeftX = float(vpWidth * colIndex),
            .TopLeftY = float(vpHeight * rowIndex),
            .Width = float(vpWidth),
            .Height = float(vpHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };

        D3D12_RECT scissorRect =
        {
            .left = LONG(vpWidth * colIndex),
            .top = LONG(vpHeight * rowIndex),
            .right = LONG(vpWidth * colIndex + vpWidth),
            .bottom = LONG(vpHeight * rowIndex + vpHeight),
        };

        cmdList->RSSetViewports(1, &viewport);
        cmdList->RSSetScissorRects(1, &scissorRect);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->DrawInstanced(3, 1, 0, 0);

        spriteRenderer.Begin(cmdList, Float2(float(vpWidth), float(vpHeight)));

        Float2 textSize = font.MeasureText(tests[testIndex].Name);
        Float2 textPos;
        textPos.x = (vpWidth * 0.5f) - (textSize.x * 0.5f),
        textPos.y = vpHeight * 0.75f,
        spriteRenderer.RenderText(cmdList, font, tests[testIndex].Name, textPos);
        spriteRenderer.End();
    }
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    SHTest app(lpCmdLine);
    app.Run();
}
