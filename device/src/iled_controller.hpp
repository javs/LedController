#pragma once

#include <functional>

#include "led_device.h"

struct ILEDController
{
    virtual LEDs::Common::LEDState GetState() const = 0;
    virtual void SetState(const LEDs::Common::LEDState& state) = 0;

    //! \tparam bool State changed due to a user interaction.
    using OnStateChanged = std::function<void(bool, const LEDs::Common::LEDState&)>;

    //! Calls delegate when state changes due to controller logic.
    virtual void SetEventDelegate(OnStateChanged& delegate) = 0;
};
