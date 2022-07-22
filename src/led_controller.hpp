#pragma once

#include <chrono>

#include <InterruptIn.h>

#include "led_component.hpp"
#include "iled_controller.hpp"


//! Drives a LED strip with warm and cool lights
class LEDController : public ILEDController
{
    //! The maximum duration that the components can be keep at high power.
    static const std::chrono::milliseconds HighPowerTimeLimit;

    //! The sum of percentages of each component that is the lower threshold for high power.
    static const float HighPowerPercentageTotal;

    LEDComponent m_Cool {D3 /*PB_3*/};
    LEDComponent m_Warm {D4 /*PB_5*/};
    mbed::InterruptIn m_Button {BUTTON1};
    int m_LimitsTimer {0};

    void UpdateLEDs();

    //! Ensures that the LEDs are not driven at high power for longer than permitted
    void EnsureLimits();

    //! \return true if the components are currently using high power
    bool InHighPower();

public:
    LEDController();

    void ToggleOn();

    LEDState GetState() override;
    void SetState(const LEDState&) override;
};
