﻿#pragma once

#include "MainWindow.g.h"

namespace winrt::LEDs::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void Show();

        HWND GetHWND() const;
        winrt::Microsoft::UI::Windowing::AppWindow GetAppWindow();
        void SendLEDsStateChangedEvent();

        void Window_Activated(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs const& args);
        void warm_slider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void cool_slider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void auto_levels_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void auto_idle_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void on_off_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void SetState(bool on, bool auto_idle, bool auto_levels, float warm, float cool);

        winrt::event_token LEDsStateChanged(winrt::LEDs::LEDsStateChangedEventHandler const& handler);
        void LEDsStateChanged(winrt::event_token const& token) noexcept;

    private:
        winrt::event< winrt::LEDs::LEDsStateChangedEventHandler> m_LEDsStateChanged;
        bool m_block_events {false};
        winrt::event_token m_size_token{};
    };
}

namespace winrt::LEDs::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
