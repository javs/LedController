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
using namespace winrt::Microsoft::Windows::System::Power;
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

    display_status_revoker = PowerManager::DisplayStatusChanged(auto_revoke, { this, &App::OnDisplayStatusChanged });
    system_suspend_status_revoker = PowerManager::SystemSuspendStatusChanged(auto_revoke, { this, &App::OnSystemSuspendStatusChanged });
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

        // Release these manually as app never destroys
        tray_icon.reset();
        led_device.reset();
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

void App::OnDisplayStatusChanged(IInspectable const&, IInspectable const&)
{
    const auto status = PowerManager::DisplayStatus();

    if (status == DisplayStatus::Off)
        led_device->SetOn(false);
    else if (status == DisplayStatus::On)
        led_device->SetOn(true);
}

void App::OnSystemSuspendStatusChanged(IInspectable const&, IInspectable const&)
{
    const auto status = PowerManager::SystemSuspendStatus();

    if (status == SystemSuspendStatus::ManualResume)
        led_device->SetOn(true);
    else if (status == SystemSuspendStatus::Entering)
        led_device->SetOn(false);
}

