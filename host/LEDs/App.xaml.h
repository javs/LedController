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

        void OnTrayClick(NotifyIcon::MouseButton button);

    private:

        winrt::LEDs::MainWindow window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
    };
}
