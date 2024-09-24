#include "pch.h"

#include <limits>
#include <stdexcept>
#include <chrono>

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

#include "LEDDevice.h"


using namespace std;
using namespace std::chrono;
using namespace winrt;

using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

using namespace winrt::Microsoft::UI::Dispatching;

using namespace LEDs::Common;


LEDDevice::State LEDDevice::State::FromUSB(const LEDs::Common::LEDState& usb_state)
{
    return {
        .on = usb_state.on,
        .user = usb_state.user,
        .auto_levels = usb_state.auto_levels,
        .warm = static_cast<float>(usb_state.warm) / numeric_limits<RawLEDComponentType>::max(),
        .cool = static_cast<float>(usb_state.cool) / numeric_limits<RawLEDComponentType>::max(),
    };
}

::LEDs::Common::LEDState LEDDevice::State::ToUSB() const
{
    return {
        .current_time = 0,
        .on = on,
        .user = user,
        .auto_levels = auto_levels,
        .warm = static_cast<RawLEDComponentType>(warm * std::numeric_limits<RawLEDComponentType>::max()),
        .cool = static_cast<RawLEDComponentType>(cool * std::numeric_limits<RawLEDComponentType>::max()),
    };
}

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
    m_last = {};
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
    m_device.InputReportReceived({ this, &LEDDevice::OnInputReportReceived });

    if (m_connected_handler)
        m_connected_handler(true);
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

IAsyncOperation<bool> LEDDevice::RequestLEDs()
{
    LEDState state{};
    state.current_time = GetTimeForDevice();

    co_return co_await SendOp(USBMessageTypes::GetLEDState, state);
}

IAsyncOperation<bool> LEDDevice::SetIdle(bool idle)
{
    auto strong_self{ get_strong() };

    // Go back to foreground to sync
    co_await wil::resume_foreground(m_dispatcher);

    if (!m_fetched)
        co_return false;

    auto state{ m_last };
    if (!state.user)
    {
        state.on = !idle;
        co_return co_await SetLEDs(state);
    }

    co_return true;
}

IAsyncOperation<bool> LEDDevice::SetLEDs(LEDState& state)
{
    if (!m_fetched)
        co_return false;

    state.current_time = GetTimeForDevice();

    co_return co_await SendOp(USBMessageTypes::SetLEDState, state);
}

IAsyncOperation<bool> LEDDevice::SetLEDs(LEDDevice::State & state)
{
    auto usb_state = state.ToUSB();
    co_return co_await SetLEDs(usb_state);
}

time_t LEDDevice::GetTimeForDevice() const
{
    auto now = system_clock::now();
    auto offset = current_zone()->get_info(now).offset;

    return system_clock::to_time_t(now + offset);
}

IAsyncOperation<bool> LEDDevice::SetLightSensorRange(uint16_t min, uint16_t max)
{
    LightSensorRange new_range{
        .max = max,
        .min = min,
    };

    co_return co_await SendOp(USBMessageTypes::SetLightSensorRange, new_range);
}

void LEDDevice::ParseInputReport(const HidInputReport& report, USBMessageTypes& msg, LEDState& state)
{
    DataReader data_reader{ DataReader::FromBuffer(report.Data()) };

    data_reader.ByteOrder(ByteOrder::LittleEndian);

    const auto length = report.Data().Length();
    if (length != USBReportInputLength +1) // add a byte for the ID
        throw runtime_error("Received USB Input Report with the wrong length: " + to_string(length));

    const auto id = data_reader.ReadByte();
    if (id != 0)
        throw runtime_error("Received wrong USB Input Report ID: " + to_string(id));

    msg = static_cast<USBMessageTypes>(data_reader.ReadByte());

    array_view<uint8_t> state_view{ reinterpret_cast<uint8_t*>(&state), sizeof(state) };

    data_reader.ReadBytes(state_view);
}

fire_and_forget LEDDevice::OnInputReportReceived(HidDevice, HidInputReportReceivedEventArgs args)
{
    auto strong_self{ get_strong() };

    try
    {
        // Go back to foreground to sync
        co_await wil::resume_foreground(m_dispatcher);

        LEDState state;
        USBMessageTypes msg;
        ParseInputReport(args.Report(), msg, state);

        switch (msg)
        {
        case USBMessageTypes::GetLEDState:
        case USBMessageTypes::SetLEDState:
        {
            m_last = state;
            m_fetched = true;
            break;
        }
        case USBMessageTypes::SetLightSensorRange:
        {
            break;
        }
        default:
            throw runtime_error("Unknown USB message received: " + to_string(static_cast<uint8_t>(msg)));
        }

        // Get/Set are responses to messages sent, so a promise is waiting fullfillment
        auto local_promise = m_op_promise.lock();
        if (local_promise)
            local_promise->set_value(true);
        // else: response without request, ignore

        if (m_changed_handler)
            m_changed_handler(State::FromUSB(m_last));
    }
    catch (exception&)
    {
        // If there is a promise, set the exception to it instead of throwing
        auto local_promise = m_op_promise.lock();
        if (local_promise)
            local_promise->set_exception(std::current_exception());
        else
            throw;
    }
}

template<typename TData>
IAsyncOperation<bool> LEDDevice::SendOp(USBMessageTypes msg, const TData& state)
{
    // dont allow simultaneous ops
    if (m_op_promise.lock())
        co_return false;

    // make a promise that will be fulfilled when the response is called, or timeout
    auto local_promise = make_shared<OpPromise>();
    m_op_promise = local_promise;
    
    auto strong_self{ get_strong() };

    if (!co_await SendReport(msg, state))
        co_return false;

    // go to the thread pool to wait for the future
    co_await winrt::resume_background();

    auto future = local_promise->get_future();

    // USB responses are fast
    if (future.wait_for(500ms) == future_status::timeout)
        throw std::runtime_error("USB timed out for op: " + to_string(static_cast<uint8_t>(msg)));

    co_return future.get();
}

template<typename TData>
IAsyncOperation<bool> LEDDevice::SendReport(USBMessageTypes msg, const TData& state)
{
    if (!m_device)
        co_return false;

    auto report = m_device.CreateOutputReport();

    DataWriter dataWriter;

    dataWriter.ByteOrder(ByteOrder::LittleEndian);

    array_view<const uint8_t> data_view{ reinterpret_cast<const uint8_t*>(&state), sizeof(state) };

    // Report Id is always the first byte
    dataWriter.WriteByte(static_cast<uint8_t>(report.Id()));
    dataWriter.WriteByte(static_cast<uint8_t>(msg));
    dataWriter.WriteBytes(data_view);

    // Fill rest of report with zeroes (note windows counts here report id, USBReportOutputLength doesnt)
    for (unsigned int i = 0; i < (USBReportOutputLength - 1 - data_view.size()); ++i)
        dataWriter.WriteByte(0);

    report.Data(dataWriter.DetachBuffer());

    auto strong_self{ get_strong() };

    auto bytes_sent = co_await m_device.SendOutputReportAsync(report);

    if (bytes_sent != report.Data().Length())
        throw std::runtime_error("Wrong number of bytes sent via USB: " + to_string(bytes_sent));

    co_return true;
}
