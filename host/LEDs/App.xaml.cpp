﻿#include "pch.h"

#include <functional>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include "NotifyIcon.h"


using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::LEDs;
using namespace winrt::LEDs::implementation;


const wchar_t* App::TrayIconPath = L"Assets/dark.ico";

App::App()
{
    InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
    {
        if (IsDebuggerPresent())
        {
            auto errorMessage = e.Message();
            __debugbreak();
        }
    });
#endif
}

void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    const auto module_handle = ::GetModuleHandle(nullptr);
    
    icon.reset(reinterpret_cast<HICON>(
        ::LoadImage(
            module_handle,
            TrayIconPath,
            IMAGE_ICON,
            0, 0,
            LR_LOADFROMFILE)));

    THROW_LAST_ERROR_IF_NULL_MSG(icon, "Failed to load app icon %wS", TrayIconPath);

    tray_icon = std::make_unique<NotifyIcon>(
        module_handle,
        icon.get(),
        L"LEDs");

    window = make<MainWindow>();
    window.LEDsStateChanged({ this, &App::OnUILEDsChanged });
    
    tray_icon->SetClickAction(bind_front(&App::OnTrayClick, this));

    led_device = make_self<LEDDevice>(window.DispatcherQueue());
    led_device->OnConnected({ this, &App::OnLEDDeviceConnected });
    led_device->OnStateChanged({ this, &App::OnLEDDeviceChanged });
    led_device->DiscoverDevice();

    // Hijack the tray icon hwnd for getting these events
    // TODO: use app sdk when fixed: https://github.com/microsoft/WindowsAppSDK/issues/2833
    ::RegisterPowerSettingNotification(tray_icon->GetHWND(), &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);
    tray_icon->AddMessageHandler(bind_front(&App::TrayMessageHandler, this));
}

fire_and_forget App::OnTrayClick(NotifyIcon::MouseButton button)
{
    switch (button)
    {
    case NotifyIcon::MouseButton::Left:
    {
        co_await led_device->RequestLEDs();

        window.Show();

        break;
    }
    case NotifyIcon::MouseButton::Right:
        window.Close();

        // Release these manually as app never releases
        led_device = nullptr;
        tray_icon.reset();
        break;
    default:
        break;
    }
}

fire_and_forget App::OnUILEDsChanged(bool on, bool auto_idle, bool auto_levels, float warm, float cold)
{
    LEDDevice::State state {
        .on = on,
        .user = !auto_idle,
        .auto_levels = auto_levels,
        .warm = warm,
        .cool = cold,
    };

    co_await led_device->SetLEDs(state);
}

fire_and_forget App::OnLEDDeviceConnected(bool on)
{
    co_await led_device->RequestLEDs();
    co_await led_device->SetIdle(false); // TODO: move out of here, reconnect on sleep
}

fire_and_forget App::OnLEDDeviceChanged(LEDDevice::State state)
{
    co_await wil::resume_foreground(window.DispatcherQueue());

    window.SetState(state.on, !state.user, state.auto_levels, state.warm, state.cool);
}

fire_and_forget App::SetIdle(bool new_idle)
{
    co_await led_device->SetIdle(new_idle);
}

LRESULT App::TrayMessageHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    // TODO: not working, event never received
    //case WM_ENDSESSION:
    //{
    //    const auto& session_end = static_cast<BOOL>(wParam);
    //    const auto& flags = lParam;
    //    
    //    //if (session_end && !(flags & ENDSESSION_LOGOFF))
    //        led_device->SetOn(false);

    //    break;
    //}
    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMRESUMESUSPEND:
            SetIdle(false);
            return 0;
        case PBT_APMSUSPEND:
            SetIdle(true);
            return 0;
        case PBT_POWERSETTINGCHANGE:
        {
            auto info = reinterpret_cast<POWERBROADCAST_SETTING*>(lParam);
            if (info->PowerSetting == GUID_MONITOR_POWER_ON)
            {
                if (info->Data[0] == 0)         // off
                {
                    SetIdle(true);
                    return 0;
                }
                else if (info->Data[0] == 1)    // on
                {
                    SetIdle(false);
                    return 0;
                }
            }
        }
        }
        break;
    }

    return 0;
}
