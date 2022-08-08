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

        void OnSystemSuspendStatusChanged(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::Foundation::IInspectable const& args);
        void OnDisplayStatusChanged(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::Foundation::IInspectable const& args);

    private:

        wil::unique_hicon icon{};
        winrt::LEDs::MainWindow window {nullptr};
        std::unique_ptr<NotifyIcon> tray_icon {};
        std::unique_ptr<LEDDevice> led_device {};

        winrt::Microsoft::Windows::System::Power::PowerManager::DisplayStatusChanged_revoker
            display_status_revoker{};
        winrt::Microsoft::Windows::System::Power::PowerManager::SystemSuspendStatusChanged_revoker
            system_suspend_status_revoker{};
    };
}
