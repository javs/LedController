#include "pch.h"

#include <limits>
#include <stdexcept>

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

using namespace winrt::Microsoft::UI::Dispatching;


LEDDevice::LEDDevice(DispatcherQueue dispatcher)
    : m_dispatcher(dispatcher)
{
}

LEDDevice::~LEDDevice()
{
    StopWatcher();
    Close();
}

void LEDDevice::OnConnected(OnLEDConnected handler)
{
    m_connected_handler = move(handler);
}

void LEDDevice::OnStateChanged(OnLEDStateChanged handler)
{
    m_changed_handler = move(handler);
}

void LEDDevice::DiscoverDevice()
{
    StopWatcher();

    auto selector = HidDevice::GetDeviceSelector(USBUsagePage, USBUsageId, USBVendorId, USBProductId);
    
    m_watcher = DeviceInformation::CreateWatcher(selector);

    m_watcher.Added({ this, &LEDDevice::OnDeviceAdded });
    m_watcher.Removed({ this, &LEDDevice::OnDeviceRemoved });

    m_watcher.Start();
}

void LEDDevice::StopWatcher()
{
    if (!m_watcher)
        return;

    m_watcher.Stop();
    m_watcher = nullptr;
}

void LEDDevice::Close()
{
    if (!m_device)
        return;
    
    m_device.Close();
    m_device = nullptr;
    m_device_id.clear();
}

fire_and_forget LEDDevice::OnDeviceAdded(DeviceWatcher sender, DeviceInformation deviceInterface)
{
    auto strong_self { get_strong() };

    // Go back to foreground to sync and for FromIdAsync to get permissions
    co_await wil::resume_foreground(m_dispatcher);
    
    // Assume only one device is ever controlled
    // Close any existing device in favor of the newly discovered
    Close();

    m_device = co_await HidDevice::FromIdAsync(deviceInterface.Id(), FileAccessMode::ReadWrite);

    if (!m_device)
    {
        // On reconnects, sometimes something grabs the device for a short period (vmware?),
        // try again after a bit
        co_await 1s;
        m_device = co_await HidDevice::FromIdAsync(deviceInterface.Id(), FileAccessMode::ReadWrite);
    }

    if (!m_device)
    {
        LOG_HR_MSG(E_FAIL, "Failed to open HID device %ls, no permission?", m_device_id.c_str());
        co_return;
    }

    m_device_id = deviceInterface.Id();
    m_device.InputReportReceived({ this, &LEDDevice::OnInputReportRecieved });

    if (m_connected_handler)
        m_connected_handler(true);

    // refresh current state
    co_await RequestLEDs();
}

fire_and_forget LEDDevice::OnDeviceRemoved(DeviceWatcher sender, DeviceInformationUpdate deviceUpdate)
{
    auto strong_self{ get_strong() };

    // Go back to foreground to sync
    co_await wil::resume_foreground(m_dispatcher);

    // Can't happen unless multiple devices ?
    if (deviceUpdate.Id() != m_device_id)
        co_return;

    Close();

    if (m_connected_handler)
        m_connected_handler(false);
}

IAsyncOperation<bool> LEDDevice::SetLEDs(bool on, float warm, float cool)
{
    const LEDState state{
        on,
        static_cast<RawLEDComponentType>(warm * std::numeric_limits<RawLEDComponentType>::max()),
        static_cast<RawLEDComponentType>(cool * std::numeric_limits<RawLEDComponentType>::max()),
    };

    co_return co_await SetLEDs(state);
}

IAsyncOperation<bool> LEDDevice::SetLEDs(const LEDState& state)
{
    co_return co_await SendOp(USBMessageTypes::SetLEDState, state);
}

IAsyncOperation<bool> LEDDevice::RequestLEDs()
{
    // get op ignores state
    co_return co_await SendOp(USBMessageTypes::GetLEDState, {});
}

IAsyncOperation<bool> LEDDevice::SetOn(bool on)
{
    auto state { m_last };
    state.on = on;
    co_return co_await SetLEDs(state);
}

fire_and_forget LEDDevice::OnInputReportRecieved(HidDevice, HidInputReportReceivedEventArgs args)
{
    auto strong_self{ get_strong() };

    // Go back to foreground to sync
    co_await wil::resume_foreground(m_dispatcher);

    DataReader data_reader { DataReader::FromBuffer(args.Report().Data()) };

    data_reader.ByteOrder(ByteOrder::LittleEndian);

    auto id = data_reader.ReadByte();
    auto msg = static_cast<USBMessageTypes>(data_reader.ReadByte());

    array_view<uint8_t> state_view{ reinterpret_cast<uint8_t*>(&m_last), sizeof(m_last) };

    data_reader.ReadBytes(state_view);

    // Convert to float 0-1 rates
    const auto warm = static_cast<float>(m_last.warm) / numeric_limits<RawLEDComponentType>::max();
    const auto cool = static_cast<float>(m_last.cool) / numeric_limits<RawLEDComponentType>::max();

    if (m_changed_handler)
        m_changed_handler(m_last.on, warm, cool);

    auto local_promise = m_op_promise.lock();
    if (local_promise)
        local_promise->set_value(true);
 }

IAsyncOperation<bool> LEDDevice::SendOp(USBMessageTypes msg, const LEDState& state)
{
    // dont allow simultaneous ops
    if (m_op_promise.lock())
        co_return false;

    // make a promise that will be fulfilled when the response is called, or timeout
    auto local_promise = make_shared<promise<bool>>();
    m_op_promise = local_promise;
    
    auto strong_self{ get_strong() };

    if (!co_await SendReport(msg, state))
        co_return false;

    // go to the thread pool to wait for the future
    co_await winrt::resume_background();

    auto future = local_promise->get_future();

    // USB responses are fast
    if (future.wait_for(500ms) == future_status::timeout)
        co_return false;

    co_return future.get();
}

IAsyncOperation<bool> LEDDevice::SendReport(USBMessageTypes msg, const LEDState& state)
{
    if (!m_device)
        co_return false;

    auto report = m_device.CreateOutputReport();

    DataWriter dataWriter;

    dataWriter.ByteOrder(ByteOrder::LittleEndian);

    array_view<const uint8_t> state_view{ reinterpret_cast<const uint8_t*>(&state), sizeof(state) };

    // Report Id is always the first byte
    dataWriter.WriteByte(static_cast<uint8_t>(report.Id()));
    dataWriter.WriteByte(static_cast<uint8_t>(msg));
    dataWriter.WriteBytes(state_view);

    report.Data(dataWriter.DetachBuffer());

    auto strong_self{ get_strong() };
    
    auto response = co_await m_device.SendOutputReportAsync(report);

    co_return response == report.Data().Length();
}
