#pragma once

#include <chrono>

#include <PinNames.h>
#include <PwmOut.h>

#include "led_device.h"


//! Represents a single component of a LED strip (red, green, cool, etc.)
class LEDComponent {
    static const std::chrono::microseconds Period;
    mbed::PwmOut m_Pin;

public:
    LEDComponent(PinName pin);

    //! \return the current duty cycle value in the 0-65535 range.
    RawLEDComponentType Get();

    //! Set the current duty cycle value in the 0-65535 range.
    void Set(RawLEDComponentType raw);

    //! \return the current duty cycle value as a percentage
    float GetPercentage();
};
