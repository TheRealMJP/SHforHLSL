//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "Exceptions.h"
#include "Containers.h"

namespace SampleFramework12
{

class Window
{

// Public types
public:

    typedef void (*MsgFunction)(void*, HWND, uint32, WPARAM, LPARAM);

// Constructor and destructor
public:

    Window(HINSTANCE hinstance,
           const wchar* name = L"SampleCommon Window",
           uint32 style = WS_CAPTION|WS_OVERLAPPED|WS_SYSMENU,
           uint32 exStyle = WS_EX_APPWINDOW,
           uint32 clientWidth = 1280,
           uint32 clientHeight = 720,
           const wchar* iconResource = nullptr,
           const wchar* smallIconResource = nullptr,
           const wchar* menuResource = nullptr,
           const wchar* accelResource = nullptr);
    ~Window();



// Public methods
public:

    HWND GetHwnd() const;
    HMENU GetMenu() const;
    HINSTANCE GetHinstance() const;
    void MessageLoop();

    bool32 IsAlive() const;
    bool32 IsMinimized() const;
    bool32 HasFocus() const;
    LONG_PTR GetWindowStyle() const;
    LONG_PTR GetExtendedStyle() const;
    void SetWindowStyle(uint32 newStyle);
    void SetExtendedStyle(uint32 newExStyle);
    void Maximize();
    void SetWindowPos(int32 posX, int32 posY);
    void GetWindowPos(int32& posX, int32& posY) const;
    void ShowWindow(bool show = true);
    void SetClientArea(int32 clientX, int32 clientY);
    void GetClientArea(int32& clientX, int32& clientY) const;
    void SetWindowTitle(const wchar* title);
    void SetScrollRanges(int32 scrollRangeX, int32 scrollRangeY, int32 posX, int32 posY);
    void SetBorderless(bool borderless);
    bool Borderless() const;
    void Destroy();

    int32 CreateMessageBox(const wchar* message, const wchar* title = nullptr, uint32 type = MB_OK);

    void RegisterMessageCallback(MsgFunction msgFunction, void* context);

    operator HWND() { return hwnd; }        //conversion operator

// Private methods
private:
    void MakeWindow(const wchar* iconResource, const wchar* smallIconResource, const wchar* menuResource);

    LRESULT MessageHandler(HWND hWnd, uint32 uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT WINAPI WndProc(HWND hWnd, uint32 uMsg, WPARAM wParam, LPARAM lParam);

// Private members
private:

    // Window properties
    HWND hwnd;                  // The window handle
    HINSTANCE hinstance;        // The HINSTANCE of the application
    std::wstring appName;       // The name of the application
    uint32 style;               // The current window style
    uint32 exStyle;             // The extended window style
    HACCEL accelTable;          // Accelerator table handle
    uint32 nonFSWidth = 0;
    uint32 nonFSHeight = 0;
    int32 nonFSPosX = 0;
    int32 nonFSPosY = 0;
    uint64 nonFSStyle = 0;
    bool borderless = false;

    struct Callback
    {
        MsgFunction Function;
        void* Context;
    };

    List<Callback> messageCallbacks;            // Message callback list
};

}