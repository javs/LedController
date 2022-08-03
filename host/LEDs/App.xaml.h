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

        winrt::Windows::Foundation::IAsyncAction OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        fire_and_forget OnLEDDeviceChange(bool on, float warm, float cool);
        fire_and_forget OnTrayClick(NotifyIcon::MouseButton button);
        fire_and_forget OnUILEDsChanged(bool on, float warm, float cold, bool automatic);
        LRESULT TrayMessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:

        wil::unique_hicon icon{};
        winrt::LEDs::MainWindow window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
        std::unique_ptr<LEDDevice> led_device {};
    };
}
