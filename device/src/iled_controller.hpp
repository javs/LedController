#pragma once

#include "led_device.h"

struct ILEDController
{
    virtual LEDs::Common::LEDState GetState() = 0;
    virtual void SetState(const LEDs::Common::LEDState& state) = 0;
};
