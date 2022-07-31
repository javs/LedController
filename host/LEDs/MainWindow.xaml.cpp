#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <microsoft.ui.xaml.window.h>
#include <wil/cppwinrt_helpers.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::LEDs::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        HWND hwnd{ nullptr };
        this->try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        auto winid = winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd);
        auto app_window = winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(winid);
        auto presenter = app_window.Presenter().as<winrt::Microsoft::UI::Windowing::OverlappedPresenter>();
        presenter.IsResizable(false);
        presenter.IsMaximizable(false);
        presenter.SetBorderAndTitleBar(true, false);
        
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
