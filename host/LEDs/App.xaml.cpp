﻿#include "pch.h"

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

fire_and_forget App::OnLaunched(LaunchActivatedEventArgs const&)
{
    tray_icon = std::make_unique<NotifyIcon>(
        ::GetModuleHandle(nullptr),
        LoadIcon(NULL, IDI_EXCLAMATION),
        L"LEDs");

    window = make<MainWindow>();

    window.LEDsStateChanged({ this, &App::OnUILEDsChanged });

    tray_icon->SetClickAction(bind_front(&App::OnTrayClick, this));

    led_device = make_unique<LEDDevice>(bind_front(&App::OnLEDDeviceChange, this));
    co_await led_device->DiscoverDevice();
}

void App::OnTrayClick(NotifyIcon::MouseButton button)
{
    switch (button)
    {
    case NotifyIcon::MouseButton::Left:
    {
        led_device->RequestLEDs();

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
            icon_rect.right - window_size.Width,
            icon_rect.top - window_size.Height,
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

void App::OnUILEDsChanged(bool on, float warm, float cold, bool automatic)
{
    led_device->SetLEDs(on, warm, cold);
}

fire_and_forget App::OnLEDDeviceChange(bool on, float warm, float cool)
{
    co_await wil::resume_foreground(window.DispatcherQueue());

    window.SetState(on, warm, cool, false);
}
