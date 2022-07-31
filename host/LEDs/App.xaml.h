#pragma once

#include "App.xaml.g.h"

#include <memory>

#include "NotifyIcon.h"

namespace winrt::LEDs::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:

        winrt::Microsoft::UI::Xaml::Window window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
    };
}
