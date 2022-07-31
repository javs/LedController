#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include "NotifyIcon.h"

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>

#include <microsoft.ui.xaml.window.h>

#include <wil/result.h>
#include <wil/cppwinrt_helpers.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace LEDs;
using namespace LEDs::implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
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

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    m_icon = std::make_unique<NotifyIcon>(nullptr, LoadIcon(NULL, IDI_EXCLAMATION), L"test");

    window = make<MainWindow>();
    window.Activate();

    m_icon->SetClickAction([&](auto m) {
        HWND hwnd{ nullptr };
        window.try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        auto winid = winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd);
        auto app_window = winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(winid);

        switch (m)
        {
        case NotifyIcon::MouseButton::Right:
            window.Close();
        default:
            if (app_window.IsVisible())
                app_window.Hide();
            else
                app_window.Show();
        }
        });

    m_icon->SetWheelAction([&](auto up) {
        // this is already in UI thread
        //co_await wil::resume_foreground(window.DispatcherQueue());
        window.as<MainWindow>()->Wheel(up > 0);
        });
}

