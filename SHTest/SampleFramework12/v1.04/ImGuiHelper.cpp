//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "PCH.h"
#include "ImGuiHelper.h"
#include "Window.h"
#include "Graphics/DX12.h"
#include "Graphics/DX12_Helpers.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/ShaderCompilation.h"
#include "Graphics/Textures.h"
#include "ImGui/imgui.h"

namespace SampleFramework12
{

namespace ImGuiHelper
{

struct ImGuiVSConstants
{
    Float4x4 ProjectionMatrix;
};

static CompiledShaderPtr VS;
static CompiledShaderPtr PS;
static ID3D12PipelineState* PSO = nullptr;
static Texture FontTexture;

#if UseAsserts_
    static uint64 CurrBeginFrame = uint64(-1);
    static uint64 CurrEndFrame = uint64(-1);
#endif

ImGuiContext* GUIContext = nullptr;

static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO();
    switch(msg)
    {
    case WM_LBUTTONDOWN:
        io.MouseDown[0] = true;
        return;
    case WM_LBUTTONUP:
        io.MouseDown[0] = false;
        return;
    case WM_RBUTTONDOWN:
        io.MouseDown[1] = true;
        return;
    case WM_RBUTTONUP:
        io.MouseDown[1] = false;
        return;
    case WM_MBUTTONDOWN:
        io.MouseDown[2] = true;
        return;
    case WM_MBUTTONUP:
        io.MouseDown[2] = false;
        return;
    case WM_MOUSEWHEEL:
        io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
        return;
    case WM_MOUSEMOVE:
        io.MousePos.x = (signed short)(lParam);
        io.MousePos.y = (signed short)(lParam >> 16);
        return;
    case WM_KEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return;
    case WM_KEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return;
    case WM_CHAR:
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter(uint16(wParam));
        return;
    }
}

void Initialize(Window& window)
{
    window.RegisterMessageCallback(WindowMessageCallback, nullptr);

    GUIContext = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.RenderDrawListsFn = nullptr;
    io.ImeWindowHandle = window.GetHwnd();

    const std::wstring shaderPath = SampleFrameworkDir() + L"Shaders\\ImGui.hlsl";
    VS = CompileFromFile(shaderPath.c_str(), "ImGuiVS", ShaderType::Vertex);
    PS = CompileFromFile(shaderPath.c_str(), "ImGuiPS", ShaderType::Pixel);

    unsigned char* pixels = nullptr;
    int32 texWidth = 0;
    int32 texHeight = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &texWidth, &texHeight);

    Create2DTexture(FontTexture, texWidth, texHeight, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, false, pixels);
    io.Fonts->TexID = ToImTextureID(FontTexture.SRV, 0);
}

void Shutdown()
{
    ImGui::DestroyContext(GUIContext);
    GUIContext = nullptr;

    FontTexture.Shutdown();

    DestroyPSOs();
}

void CreatePSOs(DXGI_FORMAT rtFormat)
{
    D3D12_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.pRootSignature = DX12::UniversalRootSignatureWithIA;
    psoDesc.VS = VS.ByteCode();
    psoDesc.PS = PS.ByteCode();
    psoDesc.RasterizerState = DX12::GetRasterizerState(RasterizerState::NoCull);
    psoDesc.BlendState = DX12::GetBlendState(BlendState::AlphaBlend);
    psoDesc.DepthStencilState = DX12::GetDepthState(DepthState::Disabled);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = rtFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.InputLayout.pInputElementDescs = inputElements;
    psoDesc.InputLayout.NumElements = ArraySize_(inputElements);
    DXCall(DX12::Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO)));
}

void DestroyPSOs()
{
    DX12::DeferredRelease(PSO);
}

void BeginFrame(uint32 displayWidth, uint32 displayHeight, float timeDelta)
{
    Assert_(CurrBeginFrame != DX12::CurrentCPUFrame);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(displayWidth), float(displayHeight));
    io.DeltaTime = timeDelta;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

    ImGui::NewFrame();

    #if UseAsserts_
        CurrBeginFrame = DX12::CurrentCPUFrame;
    #endif
}

void EndFrame(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, uint32 displayWidth, uint32 displayHeight)
{
    Assert_(CurrBeginFrame == DX12::CurrentCPUFrame);
    Assert_(CurrEndFrame != DX12::CurrentCPUFrame);

    ImGui::Render();

    PIXMarker pixMarker(cmdList, "ImGui Rendering");

    ImDrawData* drawData = ImGui::GetDrawData();

    // Get memory for vertex and index buffers
    const uint64 vbSize = sizeof(ImDrawVert) * drawData->TotalVtxCount;
    const uint64 ibSize = sizeof(ImDrawIdx) * drawData->TotalIdxCount;
    MapResult vertexMem = DX12::AcquireTempBufferMem(vbSize, 4);
    MapResult indexMem = DX12::AcquireTempBufferMem(ibSize, 4);

    // Copy and convert all vertices into a single contiguous buffer
    ImDrawVert* vertexCPUMem = reinterpret_cast<ImDrawVert*>(vertexMem.CPUAddress);
    ImDrawIdx* indexCPUMem = reinterpret_cast<ImDrawIdx*>(indexMem.CPUAddress);
    for(uint64 i = 0; i < drawData->CmdListsCount; i++)
    {
        const ImDrawList* drawList = drawData->CmdLists[i];
        memcpy(vertexCPUMem, &drawList->VtxBuffer[0], drawList->VtxBuffer.size() * sizeof(ImDrawVert));
        memcpy(indexCPUMem, &drawList->IdxBuffer[0], drawList->IdxBuffer.size() * sizeof(ImDrawIdx));
        vertexCPUMem += drawList->VtxBuffer.size();
        indexCPUMem += drawList->IdxBuffer.size();
    }

    // Setup orthographic projection matrix into our constant buffer
    ImGuiVSConstants constants;

    {
        const float L = 0.0f;
        const float R = float(displayWidth);
        const float B = float(displayHeight);
        const float T = 0.0f;

        constants.ProjectionMatrix = Float4x4(Float4(2.0f/(R-L),   0.0f,           0.0f,       0.0f),
                                              Float4(0.0f,         2.0f/(T-B),     0.0f,       0.0f),
                                              Float4(0.0f,         0.0f,           0.5f,       0.0f),
                                              Float4((R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f));
    }

    // Setup viewport
    DX12::SetViewport(cmdList, displayWidth, displayHeight);

    cmdList->OMSetRenderTargets(1, &rtv, false, nullptr);

    // Bind shader and vertex buffers
    cmdList->SetPipelineState(PSO);
    cmdList->SetGraphicsRootSignature(DX12::UniversalRootSignatureWithIA);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DX12::BindTempConstantBuffer(cmdList, constants, URS_ConstantBuffers + 0, CmdListMode::Graphics);

    D3D12_VERTEX_BUFFER_VIEW vbView = { };
    vbView.BufferLocation = vertexMem.GPUAddress;
    vbView.SizeInBytes = uint32(vbSize);
    vbView.StrideInBytes = sizeof(ImDrawVert);
    cmdList->IASetVertexBuffers(0, 1, &vbView);

    D3D12_INDEX_BUFFER_VIEW ibView = { };
    ibView.BufferLocation = indexMem.GPUAddress;
    ibView.SizeInBytes = uint32(ibSize);
    ibView.Format = DXGI_FORMAT_R16_UINT;
    cmdList->IASetIndexBuffer(&ibView);

    // Render command lists
    uint32 vtxOffset = 0;
    uint32 idxOffset = 0;
    for(uint64 cmdListIdx = 0; cmdListIdx < drawData->CmdListsCount; cmdListIdx++)
    {
        const ImDrawList* drawList = drawData->CmdLists[cmdListIdx];
        for(int32 cmdIdx = 0; cmdIdx < drawList->CmdBuffer.size(); cmdIdx++)
        {
            const ImDrawCmd* drawCmd = &drawList->CmdBuffer[cmdIdx];
            if(drawCmd->UserCallback)
            {
                drawCmd->UserCallback(drawList, drawCmd);
            }
            else
            {
                const D3D12_RECT r = { int32(drawCmd->ClipRect.x), int32(drawCmd->ClipRect.y), int32(drawCmd->ClipRect.z), int32(drawCmd->ClipRect.w) };

                if(r.left < r.right && r.top < r.bottom)
                {
                    DX12::BindTempConstantBuffer(cmdList, drawCmd->TextureId, URS_ConstantBuffers + 1, CmdListMode::Graphics);

                    cmdList->RSSetScissorRects(1, &r);

                    cmdList->DrawIndexedInstanced(drawCmd->ElemCount, 1, idxOffset, vtxOffset, 0);
                }
            }
            idxOffset += drawCmd->ElemCount;
        }
        vtxOffset += drawList->VtxBuffer.size();
    }

    #if UseAsserts_
        CurrEndFrame = DX12::CurrentCPUFrame;
    #endif
}

} // namespace ImGuiHelper

} // namespace SampleFramework12
