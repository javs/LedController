#include "pch.h"

// TODO: fix all this crap
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include <unknwn.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <sstream>

#include <windowsx.h>

#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

#include <winrt/Xaml.h>

#include "LEDDevice.h"

using namespace winrt;
using namespace Windows::Foundation;

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::Foundation::Numerics;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND _hWnd;
// This HWND will be the window handler for the XAML Island: A child window that contains XAML.  
HWND hWndXamlIsland = nullptr;
HINSTANCE _hInstance;

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    _hInstance = hInstance;

    // The main window class name.
    const wchar_t szWindowClass[] = L"LEDs";
    WNDCLASSEX windowClass = { };

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = szWindowClass;
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    windowClass.hIconSm = LoadIcon(windowClass.hInstance, IDI_APPLICATION);

    if (RegisterClassEx(&windowClass) == NULL)
    {
        MessageBox(NULL, L"Windows registration failed!", L"Error", NULL);
        return 0;
    }

    _hWnd = CreateWindow(
        szWindowClass,
        L"LEDs",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        //WS_POPUPWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    if (_hWnd == NULL)
    {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Error", NULL);
        return 0;
    }

    RegisterPowerSettingNotification(_hWnd, &GUID_SESSION_USER_PRESENCE, DEVICE_NOTIFY_WINDOW_HANDLE);
    RegisterPowerSettingNotification(_hWnd, &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);


    // Begin XAML Island section.

    // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.
    winrt::init_apartment(apartment_type::single_threaded);

    // Initialize the XAML framework's core window for the current thread.
    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();

    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    DesktopWindowXamlSource desktopSource;

    // Get handle to the core window.
    auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();

    // Parent the DesktopWindowXamlSource object to the current window.
    check_hresult(interop->AttachToWindow(_hWnd));


    // Get the new child window's HWND. 
    interop->get_WindowHandle(&hWndXamlIsland);

    RECT rect;
    GetClientRect(_hWnd, &rect);

    SetWindowPos(hWndXamlIsland, 0, 0, 0, rect.right, rect.bottom, SWP_SHOWWINDOW);

    winrt::Xaml::MainPage xamlContainer;

    desktopSource.Content(xamlContainer);

    LEDDevice device;
    device.DiscoverDevice();

    // End XAML Island section.

    ShowWindow(_hWnd, SW_SHOWNORMAL);

    //Message loop:
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT messageCode, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    wchar_t greeting[] = L"Hello World in Win32!";
    RECT rcClient;

    switch (messageCode)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CREATE:
        // Workaround for https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/issues/170
        SetForegroundWindow(GetDesktopWindow());
        SetForegroundWindow(hWnd);
        return 0;

    //// Alt workaround (doesnt work)
    //// from https://github.com/microsoft/microsoft-ui-xaml/issues/6414
    //case WM_SETFOCUS:
    //    SetFocus(hWndXamlIsland);
    //    return 0;

    case WM_POWERBROADCAST:
    {
        std::wostringstream out;
        auto t = time(nullptr);

        out << "[" << t << "] ";

        switch (wParam)
        {
        case PBT_APMPOWERSTATUSCHANGE:
            out << L"PBT_APMPOWERSTATUSCHANGE";
            break;
        case PBT_APMRESUMEAUTOMATIC:
            out << L"PBT_APMRESUMEAUTOMATIC";
            break;
        case PBT_APMRESUMESUSPEND:
            out << L"PBT_APMRESUMESUSPEND";
            break;
        case PBT_APMSUSPEND:
            out << L"PBT_APMSUSPEND";
            break;
        case PBT_POWERSETTINGCHANGE:
        {
            out << L"PBT_POWERSETTINGCHANGE - ";
            auto info = reinterpret_cast<POWERBROADCAST_SETTING*>(lParam);
            if (info->PowerSetting == GUID_MONITOR_POWER_ON)
                out << L"Monitor";
            else if (info->PowerSetting == GUID_SESSION_USER_PRESENCE)
                out << L"User Display";
            else
                out << L"Other";

            out << " Data: " << info->Data[0];
            break;
        }
        }
        //MessageBox(hWnd, out.str().c_str(), L"Msg", MB_OK);
        return 0;
    }

        // Process other messages.

    default:
        return DefWindowProc(hWnd, messageCode, wParam, lParam);
        break;
    }

    return 0;
}
