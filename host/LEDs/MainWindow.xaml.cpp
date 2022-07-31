#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

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
}
