#include <cstdio>
#include <events/mbed_events.h>
#include <DigitalOut.h>

#include "led_controller.hpp"
#include "led_device.h"
#include "settings.hpp"
#include "usb_led_device.hpp"
#include "apds9960_driver.hpp"

using namespace LEDs::Common;

int main()
{
    EventQueue *queue = mbed::mbed_event_queue();

    mbed::DigitalOut led {LED1, 1};

    printf("LED Controller Device v%hhu.%hhu\n",
        DeviceVersionMajor, DeviceVersionMinor);

    Settings::get().PrintDiags();

    APDS9960Driver light_sensor { PB_9, PB_8 };
    printf("APDS9960%s detected\n", light_sensor.Init() ? "" : " not");

    // Both pins are on the same timer by default, use the alt mode of one
    LEDController controller { PC_1, PB_0_ALT0 };

    // Blocks until host connection completes
    USBLEDDevice usb {controller};

    // Setup complete
    led = 0;

    queue->dispatch_forever();
}
