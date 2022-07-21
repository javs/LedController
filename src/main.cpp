#include <chrono>

#include <mbed.h>
#include <events/mbed_events.h>

#include "led_controller.hpp"
#include "settings.hpp"
#include "usb_led_device.hpp"

int main()
{
    EventQueue *queue = mbed_event_queue();

    DigitalOut led {LED1, 0};

    Settings::get().PrintDiags();

    LEDController controller;

    // Blocks until host connection completes
    USBLEDDevice usb {controller};

    // Setup complete
    led = 1;

    queue->dispatch_forever();
}
