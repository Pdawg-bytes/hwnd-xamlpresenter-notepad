// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "shellapi.h"
#include "roapi.h"
#include "shlobj_core.h"
#include "Objbase.h"
#include "wrl.h"
#include "ppltasks.h"
#include <Windows.UI.Core.CoreWindowFactory.h>

using namespace Microsoft::WRL;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Concurrency;

#using "C:\Users\pgago\Downloads\UpdatedShell16v2\Windows.UI.Xaml.Hosting.winmd" // Make sure to set the path to where ever your winmd file is! (This path is an example on my computer)

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        RoInitialize(RO_INIT_MULTITHREADED);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

enum WINDOW_TYPE
{
    IMMERSIVE_BODY,
    IMMERSIVE_DOCK,
    IMMERSIVE_HOSTED,
    IMMERSIVE_TEST,
    IMMERSIVE_BODY_ACTIVE,
    IMMERSIVE_DOCK_ACTIVE,
    NOT_IMMERSIVE
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(WINAPI* PrivateCreateCoreWindow)(WINDOW_TYPE, const wchar_t* title, int x, int y, int width, int height, int attributes, HWND owner, const IID& guid, void** ppv);

HMODULE uiCore = LoadLibrary(L"Windows.UI.dll");
auto privateCreateCoreWindow = (PrivateCreateCoreWindow)GetProcAddress(uiCore, MAKEINTRESOURCEA(1500));

ComPtr<ICoreWindow> coreWindow = nullptr;

ICoreDispatcher* dispatcher = nullptr;

XamlPresenter^ i;

INT ModdedShellAboutW(
    HWND    hWnd,
    LPCWSTR szApp,
    LPCWSTR szOtherStuff,
    HICON   hIcon
)
{
    const wchar_t CLASS_NAME[] = L"WinRTWindow";

    WNDCLASS wc = { };

    HWND window;

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    window = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"XamlPresenter",               // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,                   // Parent window    
        NULL,                   // Menu
        GetModuleHandle(NULL),  // Instance handle
        NULL                    // Additional application data
    );

    if (coreWindow == nullptr)
    {
        GUID iid;
        IIDFromString(L"{79B9D5F2-879E-4B89-B798-79E47598030C}", &iid);
        privateCreateCoreWindow(NOT_IMMERSIVE, L"XamlPresenter", 0, 0, 1, 1, 0, window, iid, (void**)coreWindow.GetAddressOf());
    }

    ShowWindow(window, 1);

    i = XamlPresenter::CreateFromHwnd((int)window);
    i->InitializePresenter();
    auto y = ref new Grid();
    auto z = ref new Button();
    z->Content = "Hello";
    y->Children->Append(z);
    auto h = ref new ProgressRing();
    h->IsActive = true;
    y->Children->Append(h);
    i->Content = y;
    EnableMouseInPointer(true);

    if (dispatcher == nullptr)
    {
        coreWindow->get_Dispatcher(&dispatcher);

        dispatcher->ProcessEvents(CoreProcessEventsOption::CoreProcessEventsOption_ProcessUntilQuit);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void ModdedDragAcceptFiles(
    HWND hWnd,
    BOOL fAccept
)
{
    DragAcceptFiles(hWnd, fAccept);
}

HINSTANCE ModdedShellExecuteW(
    HWND    hwnd,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT     nShowCmd
)
{
    return ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}

UINT ModdedDragQueryFileW(
    HDROP  hDrop,
    UINT   iFile,
    LPWSTR lpszFile,
    UINT   cch
)
{
    return DragQueryFileW(hDrop, iFile, lpszFile, cch);
}

void ModdedDragFinish(
    HDROP hDrop
)
{
    DragFinish(hDrop);
}

void ModdedSHAddToRecentDocs(
    UINT    uFlags,
    LPCVOID pv
)
{
    SHAddToRecentDocs(uFlags, pv);
}