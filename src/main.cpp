#include <chrono>

#include <mbed.h>
#include <platform/mbed_thread.h>

#include "settings.h"

using namespace std::chrono;
using namespace std::chrono_literals;


const microseconds Period   =   5ms;
const auto Delay            =   2ms;
const auto Increment        = 0.0005f; // %


EventQueue *queue = mbed_event_queue();

DigitalOut led(LED1);
InterruptIn button(USER_BUTTON);
PwmOut white(PB_3);
PwmOut brown(PB_5);
Settings settings{};



int main()
{
    settings.PrintDiags();

    bool state = false;

    state = settings.GetOn();

    white.period_us(Period.count());
    brown.period_us(Period.count());

    button.rise([&](){
        state = !state;

        white.pulsewidth_us(state ? Period.count() : 0);
        brown.pulsewidth_us(state ? Period.count() : 0);
        
        queue->call([&]() {
            settings.SetOn(state);
        });
    });

    float duty = 0.0f;
    float sign = 1.0f;

    // Warm-Cool Cycling
    // queue->call_every(Delay, [&]() {
    //     if (state) {
    //         duty += sign * Increment;

    //         if (duty >= 1.0f) {
    //             duty = 1.0f;
    //             sign *= -1.0f;
    //         } else if (duty <= 0.0f) {
    //             duty = 0.0f;
    //             sign *= -1.0f;
    //         }

    //         white.pulsewidth_us(Period.count() * duty);
    //         brown.pulsewidth_us(Period.count() * (1.0f - duty));
    //     } else {
    //         duty = 0.0f;
    //         white.pulsewidth_us(0);
    //         brown.pulsewidth_us(0);
    //     }
    // });

    queue->dispatch_forever();
}
