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

using namespace LEDs::Common;


LEDDevice::State::State(LEDs::Common::LEDState& usb_state)
    : on(usb_state.on)
    , warm(static_cast<float>(usb_state.warm) / numeric_limits<RawLEDComponentType>::max())
    , cool(static_cast<float>(usb_state.cool) / numeric_limits<RawLEDComponentType>::max())
{
}

LEDDevice::State::State(bool on, float warm, float cool)
    : on(on)
    , warm(warm)
    , cool(cool)
{
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

IAsyncOperation<bool> LEDDevice::SetLEDs(float warm, float cool)
{
    co_return co_await SetLEDs(m_last.on, warm, cool);
}

IAsyncOperation<bool> LEDDevice::SetLEDs(const LEDState& state)
{
    // state already matches, silently succeeed
    if (m_last == state)
        co_return true;

    if (!co_await SendOp(USBMessageTypes::SetLEDState, state))
        co_return false;

    // TODO: maybe check it set? (compare last with state)

    co_return true;
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

void LEDDevice::ParseInputReport(const HidInputReport& report, USBMessageTypes& msg, LEDState& state)
{
    DataReader data_reader{ DataReader::FromBuffer(report.Data()) };

    data_reader.ByteOrder(ByteOrder::LittleEndian);

    const auto length = report.Data().Length();
    if (length != USBReportInputLength +1) // add a byte for the ID
        throw runtime_error("Recieved USB Input Report with the wrong length: " + to_string(length));

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
            // Get/Set are responses to messages sent, so a promise is waiting fullfillment
            auto local_promise = m_op_promise.lock();
            if (local_promise)
                local_promise->set_value(true);
            // else: response without request, ignore
            break;
        }
        case USBMessageTypes::UserLEDState:
        {
            // UserLEDState is an unsolicited message, ignore promises
            break;
        }
        default:
            throw runtime_error("Unknown USB message received: " + to_string(static_cast<uint8_t>(msg)));
        }

        m_last = state;

        if (m_changed_handler)
            m_changed_handler(m_last);
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

IAsyncOperation<bool> LEDDevice::SendOp(USBMessageTypes msg, const LEDState& state)
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

    auto bytes_sent = co_await m_device.SendOutputReportAsync(report);

    if (bytes_sent != report.Data().Length())
        throw std::runtime_error("Wrong number of bytes sent via USB: " + to_string(bytes_sent));

    co_return true;
}
