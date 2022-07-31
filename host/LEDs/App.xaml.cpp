#include "pch.h"

#include <stdexcept>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include "NotifyIcon.h"

#include <winrt/Microsoft.UI.Interop.h>

#include <microsoft.ui.xaml.window.h>


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

void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    tray_icon = std::make_unique<NotifyIcon>(
        ::GetModuleHandle(nullptr),
        LoadIcon(NULL, IDI_EXCLAMATION),
        L"LEDs");

    window = make<MainWindow>();
    
    
    tray_icon->SetClickAction([&](auto m) {
        auto app_window = window.as<MainWindow>()->GetAppWindow();

        switch (m)
        {
        case NotifyIcon::MouseButton::Right:
            window.Close();
            break;
        default:
            if (app_window.IsVisible())
                app_window.Hide();
            else
                app_window.Show(true);
        }
        });

    tray_icon->SetWheelAction([&](auto up) {
        // this is already in UI thread
        //co_await wil::resume_foreground(window.DispatcherQueue());
        window.as<MainWindow>()->Wheel(up > 0);
        });
}

