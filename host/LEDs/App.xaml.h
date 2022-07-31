#pragma once

#include "App.xaml.g.h"

#include <memory>

#include "NotifyIcon.h"
#include "LEDDevice.h"

namespace winrt::LEDs::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        fire_and_forget OnLEDDeviceChange(bool on, float warm, float cool);

        fire_and_forget OnTrayClick(NotifyIcon::MouseButton button);

    private:

        winrt::LEDs::MainWindow window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
        std::unique_ptr<LEDDevice> led_device {};
    };
}
