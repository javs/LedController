#include <cstdio>

#include <events/mbed_events.h>
#include <usb/usb_phy_api.h>

#include "usb_led_device.hpp"

USBLEDDevice::USBLEDDevice(ILEDController& controller)
    : USBHID(get_usb_phy(), ReportOutputLength, ReportInputLength, VendorId, ProductId, ProductRelease)
    , m_Controller(controller)
{
    #ifdef DEVICE_USBDEVICE
        printf("USB support is on.\n");
    #endif

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

        queue->call([=]() mutable {
            const auto message_type = *(reinterpret_cast<MessageTypes*>(input_report.data));

            switch(message_type)
            {
                case MessageTypes::GetLEDState:
                {
                    auto state = m_Controller.GetState();
                    SendLEDState(state);
                    break;
                }
                case MessageTypes::SetLEDState:
                {
                    auto state =
                        *(reinterpret_cast<ILEDController::LEDState*>(
                            input_report.data + sizeof(MessageTypes)));
                    m_Controller.SetState(state);

                    // Respond back with the same state set
                    SendLEDState(state);
                    break;
                }
                default:
                    printf("Invalid USB message: %hhu", message_type);
            }
        });
    }
}

void USBLEDDevice::SendLEDState(ILEDController::LEDState state)
{
    HID_REPORT output_report{
        .length = ReportOutputLength,
        .data = {0},
    };

    // Fill type & state in the target structure
    uint8_t* offset = output_report.data;
    *(reinterpret_cast<MessageTypes*>(offset)) = MessageTypes::GetLEDState;

    offset += sizeof(MessageTypes::GetLEDState);
    *(reinterpret_cast<ILEDController::LEDState*>(offset)) = state;

    send(&output_report);
}
