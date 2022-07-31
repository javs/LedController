#pragma once

#include "MainWindow.g.h"

namespace winrt::LEDs::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        winrt::Microsoft::UI::Windowing::AppWindow GetAppWindow();

        bool Wheel();
        void Wheel(bool value);

        fire_and_forget myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::LEDs::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
