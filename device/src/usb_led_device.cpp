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
        std::bind(&USBLEDDevice::OnControllerStateChanged, this, _1, _2);
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
                    auto state = m_Controller.GetState();
                    SendUSBMessage(USBMessageTypes::SetLEDState, state);
                    break;
                }
                case USBMessageTypes::SetLEDState:
                {
                    auto state =
                        *(reinterpret_cast<LEDState*>(input_report.data + sizeof(USBMessageTypes)));
                    m_Controller.SetState(state);

                    // Respond back with the same state set
                    SendUSBMessage(USBMessageTypes::SetLEDState, state);
                    break;
                }
                case USBMessageTypes::SetTime:
                {
                    auto t = reinterpret_cast<time_t>(input_report.data + sizeof(USBMessageTypes));
                    
                    set_time(t);
                    break;
                }
                default:
                    printf("Invalid USB message: %i\n", static_cast<int>(message_type));
            }
        });
    }
}

void USBLEDDevice::SendUSBMessage(USBMessageTypes id, LEDState state)
{
    HID_REPORT output_report{
        .length = USBReportOutputLength,
        .data = {0},
    };

    // Fill type & state in the target structure
    uint8_t* offset = output_report.data;
    *(reinterpret_cast<USBMessageTypes*>(offset)) = id;

    offset += sizeof(USBMessageTypes::GetLEDState);
    *(reinterpret_cast<LEDState*>(offset)) = state;

    send(&output_report);
}

void USBLEDDevice::OnControllerStateChanged(bool user, const LEDs::Common::LEDState& state)
{
    SendUSBMessage(
        user ? USBMessageTypes::UserLEDState : USBMessageTypes::GetLEDState,
        state
        );
}
