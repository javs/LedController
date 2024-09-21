#pragma once

#include <functional>

#include "led_device.h"

struct ILEDController
{
    virtual LEDs::Common::LEDState GetState() const = 0;

    //! \param state new state to set, which may be corrected
    virtual void SetState(LEDs::Common::LEDState& state) = 0;

    using OnStateChanged = std::function<void(const LEDs::Common::LEDState&)>;

    //! Calls delegate when state changes due to controller logic.
    virtual void SetEventDelegate(OnStateChanged& delegate) = 0;

    // TODO: separate interface or mixin
    virtual void SetLightSensorRange(const LEDs::Common::LightSensorRange& range) = 0;
};
