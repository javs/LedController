#pragma once

#include <chrono>

#include <InterruptIn.h>

#include "led_device.h"
#include "led_component.hpp"
#include "iled_controller.hpp"


//! Drives a LED strip with warm and cool lights
class LEDController : public ILEDController
{
    //! The maximum duration that the components can be keep at high power.
    static const std::chrono::milliseconds HighPowerTimeLimit;

    //! The sum of percentages of each component that is the lower threshold for high power.
    static const float HighPowerPercentageTotal;

    LEDComponent m_Cool;
    LEDComponent m_Warm;
    bool m_User {false};
    mbed::InterruptIn m_Button;
    int m_LimitsTimer {0};
    OnStateChanged m_ChangedDelegate{};

    //! \param notify issue OnStateChanged if true
    void UpdateLEDs(bool notify);

    //! Ensures that the LEDs are not driven at high power for longer than permitted
    void EnsureLimits();

    //! \return true if the components are currently using high power
    bool InHighPower();

public:
    explicit LEDController(PinName cool_pin, PinName warm_pin, PinName user_button_pin);
    LEDController(const LEDController&) = delete;

    //! User toggled on/off
    void ToggleOn();

    LEDs::Common::LEDState GetState() const override;
    void SetState(LEDs::Common::LEDState&) override;

    void SetEventDelegate(OnStateChanged& delegate) override;
};
