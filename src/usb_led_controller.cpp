#include <cstdio>

#include <mbed_events.h>
#include <usb/usb_phy_api.h>

#include "usb_led_controller.hpp"

USBLEDController::USBLEDController()
    : USBHID(get_usb_phy(), ReportOutputLength, ReportInputLength, VendorId, ProductId, ProductRelease)
{
    connect();
    wait_ready();
}

USBLEDController::~USBLEDController()
{
    deinit();
}

void USBLEDController::report_rx()
{
    HID_REPORT input_report{
        .length = 0,
        .data = {0}
    };

    if (read_nb(&input_report))
    {
        auto queue = mbed::mbed_event_queue();

        queue->call([=]() {
            printf("Received USB report (length = %d): ", input_report.length);

            for (int i = 0; i < input_report.length; i++)
                printf("%d", input_report.data[i]);

            printf("\n");

            HID_REPORT output_report{
                .length = ReportOutputLength,
                .data = {0},
            };

            for (int i = 0; i < output_report.length; i++)
                output_report.data[i] = rand() & UINT8_MAX;
            
            send(&output_report);
        });
    }
}
