#include <cstdio>

#include <mbed.h>
#include <events/mbed_events.h>
#include <usb/usb_phy_api.h>

#include "usb_led_device.hpp"

using namespace LEDs::Common;

USBLEDDevice::USBLEDDevice(ILEDController& controller)
    : USBHID(
        get_usb_phy(),
        USBReportOutputLength,
        USBReportInputLength,
        USBVendorId,
        USBProductId,
        USBProductRelease)
    , m_Controller(controller)
{
#ifdef DEVICE_USBDEVICE
    printf("USB support is on.\n");
#endif

    using namespace std::placeholders;
    ILEDController::OnStateChanged delegate =
        std::bind(&USBLEDDevice::OnControllerStateChanged, this, _1);
    controller.SetEventDelegate(delegate);

    connect();
    wait_ready();
}

USBLEDDevice::~USBLEDDevice()
{
    deinit();
}

void USBLEDDevice::report_rx()
{
    HID_REPORT input_report{
        .length = 0,
        .data = {0}
    };

    if (read_nb(&input_report))
    {
        auto queue = mbed::mbed_event_queue();

        queue->call([=, this]() mutable {
            const auto message_type = *(reinterpret_cast<USBMessageTypes*>(input_report.data));

            switch(message_type)
            {
                case USBMessageTypes::GetLEDState:
                {
                    auto sent_state =
                        reinterpret_cast<LEDState*>(input_report.data + sizeof(USBMessageTypes));
                    
                    UpdateTime(sent_state->current_time);

                    auto state = m_Controller.GetState();
                    SendUSBMessage(USBMessageTypes::SetLEDState, state);
                    break;
                }
                case USBMessageTypes::SetLEDState:
                {
                    auto state =
                        reinterpret_cast<LEDState*>(input_report.data + sizeof(USBMessageTypes));

                    UpdateTime(state->current_time);
                    m_Controller.SetState(*state);

                    // Respond back with the same state set
                    SendUSBMessage(USBMessageTypes::SetLEDState, *state);
                    break;
                }
                case USBMessageTypes::SetLightSensorRange:
                {
                    auto range = 
                        reinterpret_cast<LightSensorRange*>(input_report.data + sizeof(USBMessageTypes));
                    
                    m_Controller.SetLightSensorRange(*range);
                    SendUSBMessage(USBMessageTypes::SetLightSensorRange, *range);
                }
                break;
                default:
                    printf("Invalid USB message: %i\n", static_cast<int>(message_type));
            }
        });
    }
}

void USBLEDDevice::UpdateTime(time_t current_time)
{
    if (current_time != 0 && difftime(current_time, time(nullptr)) > 5)
        set_time(current_time);
}

template<typename TBody>
void USBLEDDevice::SendUSBMessage(USBMessageTypes id, const TBody& state)
{
    HID_REPORT output_report{
        .length = USBReportOutputLength,
        .data = {0},
    };

    // Fill type & state in the target structure
    uint8_t* offset = output_report.data;
    *(reinterpret_cast<USBMessageTypes*>(offset)) = id;

    offset += sizeof(USBMessageTypes);
    *(reinterpret_cast<TBody*>(offset)) = state;

    send_nb(&output_report);
}

void USBLEDDevice::OnControllerStateChanged(const LEDs::Common::LEDState& state)
{
    SendUSBMessage(USBMessageTypes::GetLEDState, state);
}
