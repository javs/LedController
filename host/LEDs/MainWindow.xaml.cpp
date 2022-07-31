#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <microsoft.ui.xaml.window.h>
#include <wil/cppwinrt_helpers.h>


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
        app_window.Resize({ 350, 200 });
    }
        
    AppWindow MainWindow::GetAppWindow()
    {
        HWND hwnd{};
        auto window_native = this->try_as<IWindowNative>();
        
        if (!window_native)
            throw runtime_error("Failed to get window_native");

        check_hresult(window_native->get_WindowHandle(&hwnd));
        auto winid = winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd);
        auto app_window = AppWindow::GetFromWindowId(winid);

        if (!app_window)
            throw runtime_error("Failed to get AppWindow");

        return app_window;
    }

    bool MainWindow::Wheel()
    {
        return false;
    }

    void MainWindow::Wheel(bool up)
    {
        myButton().Content(box_value(up ? L"up" : L"down"));
    }

    fire_and_forget MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        co_await wil::resume_foreground(this->DispatcherQueue());
        auto h = std::to_wstring(reinterpret_cast<uint32_t>(GetDesktopWindow()));
        SetForegroundWindow(GetDesktopWindow());
        myButton().Content(box_value(h));
    }
}
