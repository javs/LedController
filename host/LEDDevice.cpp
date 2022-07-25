#include "pch.h"
#include "LEDDevice.h"
#include <sstream>

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Storage;

fire_and_forget LEDDevice::DiscoverDevice()
{
    uint16_t vendorId = 0x16c0;
    uint16_t productId = 0x07CD;
    uint16_t usagePage = 0x0000;
    uint16_t usageId = 0x0000;

    auto selector = HidDevice::GetDeviceSelector(usagePage, usageId, vendorId, productId);

    auto devices = co_await DeviceInformation::FindAllAsync(selector);

    if (devices && devices.Size())
    {
        // Open the target HID device.
        m_device = co_await HidDevice::FromIdAsync(devices.GetAt(0).Id(), FileAccessMode::ReadWrite);

        if (m_device)
        {
            m_device.InputReportReceived([](HidDevice d, HidInputReportReceivedEventArgs a) {
                std::wostringstream oss{};
                oss << "Got Report with length: " << a.Report().Data().Length() << "\n";
                MessageBox(GetDesktopWindow(), oss.str().c_str(), L"", MB_OK);
                });
            //// Input reports contain data from the device.
            //device.InputReportReceived += async(sender, args) = >
            //{
            //    HidInputReport inputReport = args.Report;
            //    IBuffer buffer = inputReport.Data;

            //    // Create a DispatchedHandler as we are interracting with the UI directly and the
            //    // thread that this function is running on might not be the UI thread; 
            //    // if a non-UI thread modifies the UI, an exception is thrown.

            //    await this.Dispatcher.RunAsync(
            //        CoreDispatcherPriority.Normal,
            //        new DispatchedHandler(() = >
            //    {
            //        info.Text += "\nHID Input Report: " + inputReport.ToString() +
            //            "\nTotal number of bytes received: " + buffer.Length.ToString();
            //    }));
            //};
        }

    }
    else
    {
        MessageBox(GetDesktopWindow(), L"No USB devices found", L"", MB_OK);
    }
}
