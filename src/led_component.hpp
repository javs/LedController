
#include <chrono>

#include <PinNames.h>
#include <PwmOut.h>

//! Represents a single component of a LED strip (red, green, cool, etc.)
class LEDComponent {
    static const std::chrono::microseconds Period;
    mbed::PwmOut m_Pin;

public:
    LEDComponent(PinName pin);

    uint16_t Get();
    void Set(uint16_t raw);
};
