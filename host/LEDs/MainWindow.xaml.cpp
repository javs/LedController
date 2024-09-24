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
using namespace winrt::Windows::Foundation;


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
        ExtendsContentIntoTitleBar(true);

        const auto hwnd = GetHWND();

        // Size to contents
        m_size_token = RootElement().SizeChanged([this](IInspectable const&, SizeChangedEventArgs const&)
            {
                RootElement().SizeChanged(m_size_token);

                const auto content_size = this->Content().ActualSize();
                const auto hwnd = GetHWND();
                const auto dpi = ::GetDpiForWindow(hwnd);
                const float scalingFactor = static_cast<float>(dpi) / 96;
                const auto app_window = GetAppWindow();

                app_window.Resize({
                    static_cast<int32_t>(content_size.x * scalingFactor),
                    static_cast<int32_t>(content_size.y * scalingFactor),
                    });
            });

        // Do not show up in taskbar
        ::SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
        
        // https://github.com/mintty/mintty/pull/984/files
        //BOOL dark = true;
        //if (::DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof dark))
        //    ::DwmSetWindowAttribute(hwnd, 19 /* ?? */, &dark, sizeof dark);
    }

    void MainWindow::Show()
    {
        // Show first so window layout is calculated (Content().UpdateLayout() doesnt work)
        Activate();

        const auto app_window = GetAppWindow();
        const auto work_area = DisplayArea::Primary().WorkArea();
        const auto window_size = app_window.ClientSize();
        constexpr auto Margin = 10; // px

        app_window.Move({
            work_area.Width - window_size.Width - Margin,
            work_area.Height - window_size.Height - Margin,
            });

        // Activate only sets focus the first time, ensure it is always set
        SetForegroundWindow(GetHWND());
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

    void MainWindow::Window_Activated(IInspectable const&, WindowActivatedEventArgs const& args)
    {
        if (args.WindowActivationState() == WindowActivationState::Deactivated)
            GetAppWindow().Hide();
    }

    void MainWindow::SendLEDsStateChangedEvent()
    {
        if (m_block_events)
            return;

        // TODO move the struct so it can be used here (but there is the idl stuff, ugh)
        m_LEDsStateChanged(
            on_off().IsOn(),
            auto_idle().IsOn(),
            auto_levels().IsOn(),
            static_cast<float>(warm_slider().Value()) / 100.0f,
            static_cast<float>(cool_slider().Value()) / 100.0f
        );
    }

    void MainWindow::SetState(bool on, bool auto_idle, bool auto_levels, float warm, float cool)
    {
        m_block_events = true;
        on_off().IsOn(on);
        this->auto_idle().IsOn(auto_idle);
        this->auto_levels().IsOn(auto_levels);
        warm_slider().Value(warm * 100.0f);
        cool_slider().Value(cool * 100.0f);
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

    void MainWindow::cool_slider_ValueChanged(IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&)
    {
        SendLEDsStateChangedEvent();
    }

    void MainWindow::auto_idle_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        if (auto_idle().IsOn())
            on_off().IsOn(true);
        SendLEDsStateChangedEvent();
    }

    void MainWindow::auto_levels_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SendLEDsStateChangedEvent();
    }

    void MainWindow::on_off_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SendLEDsStateChangedEvent();
    }
}
