#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#pragma comment(lib, "Dwmapi.lib")
#include <dwmapi.h>

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <microsoft.ui.xaml.window.h>


using namespace std;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Windowing;


namespace winrt::LEDs::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        auto app_window = GetAppWindow();
        auto presenter = app_window.Presenter().as<winrt::Microsoft::UI::Windowing::OverlappedPresenter>();

        presenter.IsResizable(false);
        presenter.IsMaximizable(false);
        presenter.SetBorderAndTitleBar(true, false);

        const auto hwnd = GetHWND();

        // Do not show up in taskbar
        ::SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
        
        // https://github.com/mintty/mintty/pull/984/files
        //BOOL dark = true;
        //if (::DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof dark))
        //    ::DwmSetWindowAttribute(hwnd, 19 /* ?? */, &dark, sizeof dark);
    }
    
    HWND MainWindow::GetHWND() const
    {
        HWND hwnd{};
        auto window_native = this->try_as<IWindowNative>();

        if (!window_native)
            throw runtime_error("Failed to get window_native");

        check_hresult(window_native->get_WindowHandle(&hwnd));

        return hwnd;
    }

    AppWindow MainWindow::GetAppWindow()
    {
        auto hwnd = GetHWND();
        auto winid = winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd);
        auto app_window = AppWindow::GetFromWindowId(winid);

        if (!app_window)
            throw runtime_error("Failed to get AppWindow");

        return app_window;
    }

    void MainWindow::DPIAwareResizeClient(float height, float width)
    {
        const auto hwnd = GetHWND();
        const auto dpi = ::GetDpiForWindow(hwnd);
        const float scalingFactor = static_cast<float>(dpi) / 96;
        const auto app_window = GetAppWindow();

        app_window.Resize({
            static_cast<int32_t>(width * scalingFactor),
            static_cast<int32_t>(height * scalingFactor),
            });
    }

    void MainWindow::Window_Activated(IInspectable const&, WindowActivatedEventArgs const& args)
    {
        if (args.WindowActivationState() == WindowActivationState::Deactivated)
            GetAppWindow().Hide();
        else
        {
            // Resize to contents
            auto content_size = this->Content().ActualSize();
            DPIAwareResizeClient(content_size.y, content_size.x);
        }
    }

    void MainWindow::SendLEDsStateChangedEvent()
    {
        if (m_block_events)
            return;

        // TODO make a struct ?
        m_LEDsStateChanged(
            on_off().IsOn(),
            warm_slider().Value() / 100.0f,
            cool_slider().Value() / 100.0f,
            auto_control().IsOn()
        );
    }

    void MainWindow::SetState(bool on, float warm, float cool, bool automatic)
    {
        m_block_events = true;
        on_off().IsOn(on);
        warm_slider().Value(warm * 100.0f);
        cool_slider().Value(cool * 100.0f);
        auto_control().IsOn(automatic);
        m_block_events = false;
    }

    winrt::event_token MainWindow::LEDsStateChanged(LEDsStateChangedEventHandler const& handler)
    {
        return m_LEDsStateChanged.add(handler);
    }

    void MainWindow::LEDsStateChanged(winrt::event_token const& token) noexcept
    {
        m_LEDsStateChanged.remove(token);
    }

    void MainWindow::warm_slider_ValueChanged(IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&)
    {
        SendLEDsStateChangedEvent();
    }

    void MainWindow::cool_slider_ValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&)
    {
        SendLEDsStateChangedEvent();
    }

    void MainWindow::auto_control_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SendLEDsStateChangedEvent();
    }

    void MainWindow::on_off_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        SendLEDsStateChangedEvent();
    }
}

