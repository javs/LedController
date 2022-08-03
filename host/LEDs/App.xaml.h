#pragma once

#include "App.xaml.g.h"

#include <memory>

#include "NotifyIcon.h"
#include "LEDDevice.h"

namespace winrt::LEDs::implementation
{
    struct App : AppT<App>
    {
        static const wchar_t* TrayIconPath;
        App();

        fire_and_forget OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        fire_and_forget OnLEDDeviceChange(bool on, float warm, float cool);
        void OnTrayClick(NotifyIcon::MouseButton button);
        void OnUILEDsChanged(bool on, float warm, float cold, bool automatic);

    private:

        wil::unique_hicon icon{};
        winrt::LEDs::MainWindow window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
        std::unique_ptr<LEDDevice> led_device {};
    };
}
