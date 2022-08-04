#include "pch.h"

#include <functional>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include "NotifyIcon.h"


using namespace std;
using namespace winrt;
using namespace Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace winrt::Microsoft::UI::Windowing;
using namespace LEDs;
using namespace LEDs::implementation;


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

IAsyncAction App::OnLaunched(LaunchActivatedEventArgs const&)
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

    led_device = make_unique<LEDDevice>(bind_front(&App::OnLEDDeviceChange, this));
    co_await led_device->DiscoverDevice();

    // Hijack the tray icon hwnd for getting these events
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

        // TODO: rework this block
        const auto main_window = window.as<MainWindow>();
        const auto app_window = main_window->GetAppWindow();

        RECT icon_rect{};
        tray_icon->GetNotifyIconRect(icon_rect);

        // Show first so window layout is calculated (window.Content().UpdateLayout() doesnt work)
        app_window.Show(true);

        // Activate only works the first time, ensure the focus is always set
        SetForegroundWindow(main_window->GetHWND());
            
        const auto window_size = app_window.ClientSize();

        app_window.Move({
            icon_rect.left + (icon_rect.right - icon_rect.left) / 2 - window_size.Width / 2,
            icon_rect.top - window_size.Height - 40,
            });

        break;
    }
    case NotifyIcon::MouseButton::Right:
        window.Close();
        break;
    default:
        break;
    }
}

fire_and_forget App::OnUILEDsChanged(bool on, float warm, float cold, bool automatic)
{
    co_await led_device->SetLEDs(on, warm, cold);
}

fire_and_forget App::OnLEDDeviceChange(bool on, float warm, float cool)
{
    co_await wil::resume_foreground(window.DispatcherQueue());

    window.SetState(on, warm, cool, false);
}

LRESULT App::TrayMessageHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMRESUMESUSPEND:
            led_device->SetOn(true);
            return 0;
        case PBT_APMSUSPEND:
            led_device->SetOn(false);
            return 0;
        case PBT_POWERSETTINGCHANGE:
        {
            auto info = reinterpret_cast<POWERBROADCAST_SETTING*>(lParam);
            if (info->PowerSetting == GUID_MONITOR_POWER_ON)
            {
                if (info->Data[0] == 0)         // off
                {
                    led_device->SetOn(false);
                    return 0;
                }
                else if (info->Data[0] == 1)    // on
                {
                    led_device->SetOn(true);
                    return 0;
                }
            }
        }
        }
    }

    return 0;
}
