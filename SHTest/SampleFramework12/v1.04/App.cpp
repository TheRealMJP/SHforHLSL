//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "PCH.h"

#include "App.h"
#include "Exceptions.h"
#include "Graphics\\Profiler.h"
#include "Graphics\\Spectrum.h"
#include "Graphics\\ShaderDebug.h"
#include "SF12_Math.h"
#include "FileIO.h"
#include "Settings.h"
#include "ImGuiHelper.h"
#include "ImGui/imgui.h"
#include "Input.h"

// AppSettings framework
namespace AppSettings
{
    void Initialize();
    void Shutdown();
    void Update(uint32 displayWidth, uint32 displayHeight, const SampleFramework12::Float4x4& viewMatrix);
    void UpdateCBuffer();
}

namespace SampleFramework12
{

App* GlobalApp = nullptr;

App::App(const wchar* appName, const wchar* cmdLine) : window(nullptr, appName, WS_OVERLAPPEDWINDOW,
                                                                     WS_EX_APPWINDOW, 1280, 720),
                                                              applicationName(appName)

{
    GlobalApp = this;

    wchar exePath[MAX_PATH] = { };
    GetModuleFileName(nullptr, exePath, ArraySize_(exePath));
    const std::wstring exeDir = GetDirectoryFromFilePath(exePath);

    wchar expectedCurrDirectory[MAX_PATH] = { };
    GetFullPathName((exeDir + L"..\\..").c_str(), ArraySize_(expectedCurrDirectory), expectedCurrDirectory, nullptr);

    SetCurrentDirectory(expectedCurrDirectory);

    for(uint32 i = 0; i < NumTimeDeltaSamples; ++i)
        timeDeltaBuffer[i] = 0.0f;

    SampledSpectrum::Init();

    ParseCommandLine(cmdLine);
}

App::~App()
{

}

int32 App::Run()
{
    try
    {

        Initialize_Internal();

        AfterReset_Internal();

        CreatePSOs_Internal();

        while(window.IsAlive())
        {
            if(!window.IsMinimized())
            {
                Update_Internal();

                Render_Internal();
            }

            window.MessageLoop();
        }
    }
    catch(SampleFramework12::Exception exception)
    {
        exception.ShowErrorMessage();
        return -1;
    }

    Shutdown_Internal();

    return returnCode;
}

void App::CalculateFPS()
{
    timeDeltaBuffer[currentTimeDeltaSample] = appTimer.DeltaSecondsF();
    currentTimeDeltaSample = (currentTimeDeltaSample + 1) % NumTimeDeltaSamples;

    double averageDelta = 0;
    for(uint32 i = 0; i < NumTimeDeltaSamples; ++i)
        averageDelta += timeDeltaBuffer[i];
    averageDelta /= NumTimeDeltaSamples;

    avgFrameTime = averageDelta;
    avgFPS = uint32((1.0f / averageDelta) + 0.5);
}

void App::OnWindowResized(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(msg != WM_SIZE)
        return;

    App* app = reinterpret_cast<App*>(context);

    if(wParam != SIZE_MINIMIZED)
    {
        int width, height;
        app->window.GetClientArea(width, height);

        if(uint32(width) != app->swapChain.Width() || uint32(height) != app->swapChain.Height())
        {
            app->BeforeReset_Internal();

            app->swapChain.SetWidth(width);
            app->swapChain.SetHeight(height);
            app->swapChain.Reset();

            app->AfterReset_Internal();
        }
    }
}

void App::Exit()
{
    window.Destroy();
}

void App::BeforeFlush()
{
}

void App::ParseCommandLine(const wchar* cmdLine)
{
    if(cmdLine == nullptr)
        return;

    std::string cmdLineA = WStringToAnsi(cmdLine);
    List<std::string> parts;
    Split(cmdLineA, parts, " ");

    uint64 numParts = parts.Count();
    if(numParts == 0)
        return;

    char appString[4] = "App";

    Array<char*> partStrings(numParts + 1);
    partStrings[0] = appString;
    for(uint64 i = 0; i < numParts; ++i)
        partStrings[i + 1] = &parts[i].front();

    int32 argc = int32(numParts + 1);
    char** argv = partStrings.Data();

    cxxopts::Options options("App", "");
    options.allow_unrecognised_options();
    options.add_options()
         ("a,adapter", "GPU adapter index", cxxopts::value<int32>());

    cxxopts::ParseResult parseResult = options.parse(argc, argv);

    if(parseResult.count("adapter"))
        adapterIdx = parseResult["adapter"].as<int32>();
}

void App::Initialize_Internal()
{
    DX12::Initialize(minFeatureLevel, adapterIdx);

    window.SetClientArea(swapChain.Width(), swapChain.Height());
    swapChain.Initialize(window);

    if(showWindow)
        window.ShowWindow();

    // Create a font + SpriteRenderer
    font.Initialize(L"Consolas", 18, SpriteFont::Regular, true);
    spriteRenderer.Initialize();

    Profiler::GlobalProfiler.Initialize();

    window.RegisterMessageCallback(OnWindowResized, this);

    // Initialize ImGui
    ImGuiHelper::Initialize(window);

    ShaderDebug::Initialize();

    AppSettings::Initialize();

    Initialize();
}

void App::Shutdown_Internal()
{
    BeforeFlush();
    DX12::FlushGPU();

    DestroyPSOs();
    ImGuiHelper::Shutdown();
    ShaderDebug::Shutdown();
    ShutdownShaders();
    spriteRenderer.Shutdown();
    font.Shutdown();
    swapChain.Shutdown();
    AppSettings::Shutdown();
    Profiler::GlobalProfiler.Shutdown();

    Shutdown();

    DX12::Shutdown();
}

void App::Update_Internal()
{
    appTimer.Update();

    const uint32 displayWidth = swapChain.Width();
    const uint32 displayHeight = swapChain.Height();
    ImGuiHelper::BeginFrame(displayWidth, displayHeight, appTimer.DeltaSecondsF());

    CalculateFPS();

    AppSettings::Update(displayWidth, displayHeight, appViewMatrix);

    AppSettings::UpdateCBuffer();

    Update(appTimer);
}

void App::Render_Internal()
{
    if(UpdateShaders(false))
    {
        DestroyPSOs();

        DX12::FlushGPU();

        CreatePSOs();
    }

    DX12::BeginFrame();
    swapChain.BeginFrame();

    const POINT mousePos = MouseState::GetCursorPos(window.GetHwnd());
    ShaderDebug::BeginRender(DX12::CmdList, mousePos.x, mousePos.y);

    Render(appTimer);

    // Update the profiler
    const uint32 displayWidth = swapChain.Width();
    const uint32 displayHeight = swapChain.Height();
    Profiler::GlobalProfiler.EndFrame(displayWidth, displayHeight, avgFPS, avgFrameTime);

    DrawLog();

    if(showGUI)
        ImGuiHelper::EndFrame(DX12::CmdList, swapChain.BackBuffer().RTV, displayWidth, displayHeight);

    ShaderDebug::EndRender(DX12::CmdList);

    swapChain.EndFrame();

    DX12::EndFrame(swapChain.D3DSwapChain(), swapChain.NumVSYNCIntervals());
}

void App::BeforeReset_Internal()
{
    BeforeFlush();

    // Need this in order to resize the swap chain
    DX12::FlushGPU();

    BeforeReset();
}

void App::AfterReset_Internal()
{
    AfterReset();
}

void App::CreatePSOs_Internal()
{
    spriteRenderer.CreatePSOs(swapChain.Format(), 1);
    ImGuiHelper::CreatePSOs(swapChain.Format());

    CreatePSOs();
}

void App::DestroyPSOs_Internal()
{
    spriteRenderer.DestroyPSOs();
    ImGuiHelper::DestroyPSOs();

    DestroyPSOs();
}

void App::DrawLog()
{
    bool forceGPUTab = false;
    if(newLogMessageGPU && autoShowGPULog && showLog == false)
    {
        showLog = true;
        forceGPUTab = true;
    }
    newLogMessageGPU = false;

    const uint32 displayWidth = swapChain.Width();
    const uint32 displayHeight = swapChain.Height();

    if(showLog == false)
    {
        ImGui::SetNextWindowSize(ImVec2(75.0f, 25.0f));
        ImGui::SetNextWindowPos(ImVec2(25.0f, displayHeight - 50.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
        if(ImGui::Begin("log_button", nullptr, flags))
        {
            if(ImGui::Button("Log"))
                showLog = true;
        }

        ImGui::PopStyleVar();

        ImGui::End();

        return;
    }

    ImVec2 initialSize = ImVec2(displayWidth * 0.5f, float(displayHeight) * 0.25f);
    ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10.0f, displayHeight - initialSize.y - 10.0f), ImGuiCond_FirstUseEver);

    if(ImGui::Begin("Log", &showLog) == false)
    {
        ImGui::End();
        return;
    }

    if(ImGui::BeginTabBar("Log Tabs", ImGuiTabBarFlags_None))
    {
        if(ImGui::BeginTabItem("CPU"))
        {
            const uint64 numMessages = numLogMessages;
            const uint64 start = numMessages > MaxLogMessages ? numMessages - MaxLogMessages : 0;
            for(uint64 i = start; i < numMessages; ++i)
                ImGui::TextUnformatted(logMessages[i % MaxLogMessages].c_str());

            if(newLogMessage)
                ImGui::SetScrollHere();

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("GPU", nullptr, forceGPUTab ? ImGuiTabItemFlags_SetSelected : 0))
        {
            ImGui::Checkbox("Clear Log Every Frame", &clearGPULogEveryFrame);
            ImGui::SameLine();
            ImGui::Checkbox("Pause Log", &pauseGPULog);
            ImGui::SameLine();
            ImGui::Checkbox("Auto-Show GPU Log", &autoShowGPULog);
            ImGui::SameLine();
            const bool copyLogToClipboard = ImGui::Button("Copy Log To Clipboard");
            ImGui::Separator();

            if(copyLogToClipboard)
                ImGui::LogToClipboard();

            const uint64 numMessages = numLogMessagesGPU;
            const uint64 start = numMessages > MaxLogMessages ? numMessages - MaxLogMessages : 0;
            for(uint64 i = start; i < numMessages; ++i)
                ImGui::TextUnformatted(logMessagesGPU[i % MaxLogMessages].c_str());

            if(copyLogToClipboard)
                ImGui::LogFinish();

            if(newLogMessage && clearGPULogEveryFrame == false)
                ImGui::SetScrollHere();

            if(clearGPULogEveryFrame && pauseGPULog == false)
                numLogMessagesGPU = 0;

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    newLogMessage = false;
}

void App::AddToLog(const char* msg)
{
    if(msg == nullptr)
        return;

    const uint64 idx = uint64(InterlockedIncrement64(&numLogMessages) - 1) % MaxLogMessages;
    logMessages[idx] = msg;

    newLogMessage = true;
}

void App::AddToGPULog(const char* msg)
{
    if(msg == nullptr || pauseGPULog)
        return;

    logMessagesGPU[numLogMessagesGPU] = msg;
    numLogMessagesGPU = (numLogMessagesGPU + 1) % MaxLogMessages;

    newLogMessageGPU = true;
}

}