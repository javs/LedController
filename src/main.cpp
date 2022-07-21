#include <chrono>

#include <mbed.h>
#include <events/mbed_events.h>

#include "led_component.hpp"
#include "settings.hpp"
#include "usb_led_controller.hpp"




USBLEDController::LEDState USBGetState(){
    return USBLEDController::LEDState(2, 7);
}

void USBSetState(const USBLEDController::LEDState& state) {
    printf("set state %d %d\n", state.first, state.second);
}


int main()
{
    DigitalOut led(LED1);
    // InterruptIn button(BUTTON1);


    // Setup
    //
    EventQueue *queue = mbed_event_queue();

    led = 0;

    Settings::get().PrintDiags();

    LEDComponent cool(D3 /*PB_3*/);
    LEDComponent warm(D4 /*PB_5*/);

    #ifdef DEVICE_USBDEVICE
        printf("USB support is on.\n");
    #endif

    // state = settings.GetOn();

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

    // Blocks until host connection
    USBLEDController usb {USBGetState, USBSetState};
    led = 1;

    queue->dispatch_forever();
}
