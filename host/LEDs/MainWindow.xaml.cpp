#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <microsoft.ui.xaml.window.h>
#include <wil/cppwinrt_helpers.h>
#include <wil/resource.h>

#include "undoc.h"

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

        wil::unique_hmodule lib_user32{ ::LoadLibraryEx(L"user32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32) };

        FAIL_FAST_LAST_ERROR_IF_NULL(lib_user32);
        
        auto SetWindowCompositionAttribute = reinterpret_cast<PFN_SET_WINDOW_COMPOSITION_ATTRIBUTE>(::GetProcAddress(lib_user32.get(), "SetWindowCompositionAttribute"));
        
        FAIL_FAST_LAST_ERROR_IF(!SetWindowCompositionAttribute);

        ACCENT_POLICY policy = {
            ACCENT_ENABLE_ACRYLICBLURBEHIND,
            2,
            0x11111111,
            0
        };

        const WINDOWCOMPOSITIONATTRIBDATA data = {
            WCA_ACCENT_POLICY,
            &policy,
            sizeof(policy)
        };

        FAIL_FAST_IF_WIN32_BOOL_FALSE(SetWindowCompositionAttribute(GetHWND(), &data));
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

    void MainWindow::DPIAwareResizeClient(int height, int width)
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

    void MainWindow::Window_Activated(IInspectable const& sender, WindowActivatedEventArgs const& args)
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
