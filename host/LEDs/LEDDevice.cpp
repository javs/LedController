#include "pch.h"

#include <limits>
#include <stdexcept>

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

#include "LEDDevice.h"


using namespace std;
using namespace winrt;
using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;


LEDDevice::LEDDevice(OnLEDStateChange handler)
    : m_handler(move(handler))
{
}

LEDDevice::~LEDDevice()
{
    if (m_device)
    {
        m_device.Close();
        m_device = nullptr;
    }
}

IAsyncAction LEDDevice::DiscoverDevice(bool refresh_state)
{
    auto selector = HidDevice::GetDeviceSelector(USBUsagePage, USBUsageId, USBVendorId, USBProductId);

    auto devices = co_await DeviceInformation::FindAllAsync(selector);

    if (devices && devices.Size())
    {
        // Open the target HID device.
        m_device = co_await HidDevice::FromIdAsync(devices.GetAt(0).Id(), FileAccessMode::ReadWrite);

        if (m_device)
        {
            m_device.InputReportReceived({ this, &LEDDevice::OnInputReportRecieved });

            if (refresh_state)
            {
                co_await RequestLEDs();
                co_await 1s; // Change for awaiting the confirmation msg
            }
        }
    }
}

IAsyncAction LEDDevice::SetLEDs(bool on, float warm, float cool)
{
    const LEDState state{
        on,
        static_cast<RawLEDComponentType>(warm * std::numeric_limits<RawLEDComponentType>::max()),
        static_cast<RawLEDComponentType>(cool * std::numeric_limits<RawLEDComponentType>::max()),
    };

    co_await SendReport(USBMessageTypes::SetLEDState, state);
}

winrt::Windows::Foundation::IAsyncAction LEDDevice::SetLEDs(const LEDState& state)
{
    co_await SendReport(USBMessageTypes::SetLEDState, state);
}

IAsyncAction LEDDevice::RequestLEDs()
{
    // state is ignored for get
    co_await SendReport(USBMessageTypes::GetLEDState, {});
}

IAsyncAction LEDDevice::SetOn(bool on)
{
    auto state { m_last };
    state.on = on;
    co_await SetLEDs(state);
}

void LEDDevice::OnInputReportRecieved(
    winrt::Windows::Devices::HumanInterfaceDevice::HidDevice,
    winrt::Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs args)
{
    DataReader data_reader { DataReader::FromBuffer(args.Report().Data()) };

    data_reader.ByteOrder(ByteOrder::LittleEndian);

    auto id = data_reader.ReadByte();
    auto msg = static_cast<USBMessageTypes>(data_reader.ReadByte());

    array_view<uint8_t> state_view{ reinterpret_cast<uint8_t*>(&m_last), sizeof(m_last) };

    data_reader.ReadBytes(state_view);

    // Convert to float 0-1 rates
    const auto warm = static_cast<float>(m_last.warm) / numeric_limits<RawLEDComponentType>::max();
    const auto cool = static_cast<float>(m_last.cool) / numeric_limits<RawLEDComponentType>::max();

    // Block the set return code to avoid re-setting state
    // TODO: handle failures to set?
    if (m_handler && msg != USBMessageTypes::SetLEDState)
        m_handler(m_last.on, warm, cool);
 }

IAsyncAction LEDDevice::SendReport(USBMessageTypes msg, const LEDState& state)
{
    if (!m_device)
        throw runtime_error("No USB device discovered");

    auto report = m_device.CreateOutputReport();

    DataWriter dataWriter;

    dataWriter.ByteOrder(ByteOrder::LittleEndian);

    array_view<const uint8_t> state_view{ reinterpret_cast<const uint8_t*>(&state), sizeof(state) };

    // Report Id is always the first byte

    dataWriter.WriteByte(report.Id());
    dataWriter.WriteByte(static_cast<uint8_t>(msg));
    dataWriter.WriteBytes(state_view);

    report.Data(dataWriter.DetachBuffer());

    auto response = co_await m_device.SendOutputReportAsync(report);
}
