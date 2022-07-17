#include <chrono>

#include <mbed.h>
#include <platform/mbed_thread.h>
#include <usb/USBHID.h>

#include "settings.h"

using namespace std::chrono;
using namespace std::chrono_literals;


const microseconds Period   =   5ms;
const auto Delay            =   2ms;
const auto Increment        = 0.0005f; // %


//EventQueue *queue = mbed_event_queue();

DigitalOut led(LED1);
InterruptIn button(USER_BUTTON);
PwmOut cool(D3 /*PB_3*/);
PwmOut warm(D4 /*PB_5*/);
//Settings settings{};


HID_REPORT output_report = {
    .length = 8,
    .data = {0}
};
HID_REPORT input_report = {
    .length = 0,
    .data = {0}
};

int main()
{   
    // settings.PrintDiags();

    // bool state = false;

    // state = settings.GetOn();

    // float warm_per = 0.5f;
    // float cool_per = 0.4f;
    // //float sign = 1.0f;

    // cool.period_us(Period.count());
    // warm.period_us(Period.count());

    // cool.pulsewidth_us(state ? Period.count() * cool_per : 0);
    // warm.pulsewidth_us(state ? Period.count() * warm_per : 0);

    // button.rise([&](){
    //     state = !state;

    //     cool.pulsewidth_us(state ? Period.count() * cool_per : 0);
    //     warm.pulsewidth_us(state ? Period.count() * warm_per : 0);
        
    //     queue->call([&]() {
    //         settings.SetOn(state);
    //     });
    // });

    // queue->dispatch_forever();

    
    led = 1;

    const chrono::microseconds USBEnumDelay = 500ms;

    {
        DigitalIn dplus{PA_12};
        dplus.mode(PullUp);
        wait_us(USBEnumDelay.count());
    }

    led = 0;

    #ifdef DEVICE_USBDEVICE
        printf("USB on\n");
    #endif

    USBHID HID(true, 8, 8, 0x1234, 0x0006, 0x0001); // PA_11 USB_DM / PA_12 USB_DP
    led = 1;

    while (1)
    {

        // Fill the report
        for (int i = 0; i < output_report.length; i++)
        {
            output_report.data[i] = rand() & UINT8_MAX;
        }

        // Send the report
        HID.send(&output_report);

        // Try to read a msg
        if (HID.read_nb(&input_report))
        {
            led = !led;
            for (int i = 0; i < input_report.length; i++)
            {
                printf("%d ", input_report.data[i]);
            }
            printf("\r\n");
        }
    }
}
