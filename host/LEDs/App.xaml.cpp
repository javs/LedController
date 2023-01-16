#include "pch.h"

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

    THROW_LAST_ERROR_IF_NULL_MSG(icon, "Failed to load app icon %S", TrayIconPath);

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

    window.SetAutomatic(true);

    temp_manager.OnUpdated({ this, &App::OnTempUpdated });
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

fire_and_forget App::OnUILEDsChanged(bool on, float warm, float cold, bool automatic)
{
    temp_manager.Enable(automatic);

    if (!automatic)
        co_await led_device->SetLEDs(on, warm, cold);
    else
        ReapplyDevice();
}

void App::OnLEDDeviceConnected(bool on)
{
    // a device connected, refresh it based on the expected state
    if (on)
        ReapplyDevice();
}

fire_and_forget App::OnLEDDeviceChanged(LEDDevice::State state)
{
    co_await wil::resume_foreground(window.DispatcherQueue());

    window.SetState(state.on, state.warm, state.cool);
}

void App::SetIdle(bool new_idle)
{
    idle = new_idle;

    ReapplyDevice();
}

fire_and_forget App::OnTempUpdated(float warm, float cool)
{
    bool result = false;
    try
    {
        result = co_await led_device->SetLEDs(!idle, warm, cool);
    }
    catch (...) {} // retry on any exception

    // On failure, try once more
    if (!result)
    {
        co_await 500ms;
        try {
            co_await led_device->SetLEDs(!idle, warm, cool);
        } catch (...) {}; // give up until next update
    }
}

fire_and_forget App::ReapplyDevice()
{
    // Only update the temp manager for auto mode, otherwise keep device as is
    // Only auto mode can get out sync, manual mode just uses the current device reported state

    // temp_manager requires main thread
    co_await wil::resume_foreground(window.DispatcherQueue());
    temp_manager.Update();
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
