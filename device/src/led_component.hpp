#pragma once

#include <chrono>

#include <PinNames.h>
#include <PwmOut.h>

#include "led_device.h"


//! Represents a single component of a LED strip (red, green, cool, etc.)
class LEDComponent {
    static const std::chrono::microseconds Period;

    //! Light percentage to change per update tick.
    static const float ChangePerTick;

    //! Time between change while updating led pin.
    static const std::chrono::milliseconds ChangeDelay;

    //! Difference from setpoint to current state to not consider update.
    static const float UpdateCutoff;

    mbed::PwmOut m_Pin;
    float m_SetPoint;
    int m_UpdateTimer{};

    //! Called when it's time to update the pin to get closer to the setpoint.
    void UpdatePin();

    //! Start/Stop the update timer as needed.
    void CheckForUpdate();

public:
    explicit LEDComponent(PinName pin);

    //! \return the current duty cycle value in the 0-65535 range.
    LEDs::Common::RawLEDComponentType Get();

    //! Set the current duty cycle value in the 0-65535 range.
    void Set(LEDs::Common::RawLEDComponentType raw);

    //! \return the current duty cycle value as a percentage
    float GetPercentage();

    //! \return true if the pin doesn't match the setpoint, and is updating to do so.
    bool IsUpdating();
};
