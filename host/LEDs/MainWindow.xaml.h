#pragma once

#include "MainWindow.g.h"

namespace winrt::LEDs::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        HWND GetHWND() const;
        winrt::Microsoft::UI::Windowing::AppWindow GetAppWindow();

        void DPIAwareResizeClient(int height, int width);

        void Window_Activated(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs const& args);
    };
}

namespace winrt::LEDs::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
