#include <cstdio>
#include <events/mbed_events.h>
#include <DigitalOut.h>

#include "auto_led_controller.hpp"
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

    {
        auto detected = light_sensor.Init();
        printf("APDS9960%s detected\n", detected ? "" : " not");
    }

    // Both LED pins are on the same timer by default, use the alt mode of one
    AutoLEDController controller { PC_1, PB_0_ALT0, BUTTON1, light_sensor };

    // Blocks until host connection completes
    USBLEDDevice usb {controller};

    // Setup complete
    led = 0;

    queue->dispatch_forever();
}
