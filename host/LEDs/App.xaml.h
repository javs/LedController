#pragma once

#include "App.xaml.g.h"

#include <memory>

#include "NotifyIcon.h"
#include "LEDDevice.h"
#include "TempManager.h"

namespace winrt::LEDs::implementation
{
    struct App : AppT<App>
    {
        static const wchar_t* TrayIconPath;
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        fire_and_forget OnTrayClick(NotifyIcon::MouseButton button);

        void OnLEDDeviceConnected(bool connected);
        fire_and_forget OnLEDDeviceChanged(bool on, float warm, float cool);

        fire_and_forget OnUILEDsChanged(bool on, float warm, float cold, bool automatic);
        LRESULT TrayMessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:

        wil::unique_hicon icon{};
        winrt::LEDs::MainWindow window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
        winrt::com_ptr<LEDDevice> led_device {};
        TempManager temp_manager{};
    };
}
