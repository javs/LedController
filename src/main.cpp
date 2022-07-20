#include <chrono>

#include <mbed.h>
#include <events/mbed_events.h>
#include <platform/mbed_thread.h>

#include "usb_led_controller.hpp"
#include "settings.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;


const microseconds Period   =   5ms;
const auto Delay            =   2ms;
const auto Increment        = 0.0005f; // %


DigitalOut led(LED1);
InterruptIn button(BUTTON1);
PwmOut cool(D3 /*PB_3*/);
PwmOut warm(D4 /*PB_5*/);
Settings settings{};

USBLEDController::LEDState USBGetState(){
    return USBLEDController::LEDState(2, 7);
}

void USBSetState(const USBLEDController::LEDState& state) {
    printf("set state %d %d\n", state.first, state.second);
}


int main()
{
    // Setup
    //
    EventQueue *queue = mbed_event_queue();

    led = 0;

    #ifdef DEVICE_USBDEVICE
        printf("USB support is on.\n");
    #endif

    // settings.PrintDiags();

    // bool state = false;

    // state = settings.GetOn();

    // float warm_per = 0.5f;
    // float cool_per = 0.4f;
    // //float sign = 1.0f;

    // cool.period_us(Period.count());
    // warm.period_us(Period.count());

    // button.rise([&](){
    //     state = !state;

    //     cool.pulsewidth_us(state ? Period.count() * cool_per : 0);
    //     warm.pulsewidth_us(state ? Period.count() * warm_per : 0);
        
    //     queue->call([&]() {
    //         settings.SetOn(state);
    //     });
    // });

    // // Initial state
    // //
    // cool.pulsewidth_us(state ? Period.count() * cool_per : 0);
    // warm.pulsewidth_us(state ? Period.count() * warm_per : 0);

    // block for usb
    // Uses PA_11 USB_DM / PA_12 USB_DP
    USBLEDController usb {USBGetState, USBSetState};
    led = 1;

    queue->dispatch_forever();
}
