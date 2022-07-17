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


EventQueue *queue = mbed_event_queue();

DigitalOut led(LED1);
InterruptIn button(USER_BUTTON);
PwmOut cool(D3 /*PB_3*/);
PwmOut warm(D4 /*PB_5*/);
Settings settings{};

int main()
{   
    settings.PrintDiags();

    bool state = false;

    state = settings.GetOn();

    float warm_per = 0.5f;
    float cool_per = 0.4f;
    //float sign = 1.0f;

    cool.period_us(Period.count());
    warm.period_us(Period.count());

    cool.pulsewidth_us(state ? Period.count() * cool_per : 0);
    warm.pulsewidth_us(state ? Period.count() * warm_per : 0);

    button.rise([&](){
        state = !state;

        cool.pulsewidth_us(state ? Period.count() * cool_per : 0);
        warm.pulsewidth_us(state ? Period.count() * warm_per : 0);
        
        queue->call([&]() {
            settings.SetOn(state);
        });
    });

    queue->dispatch_forever();
}
