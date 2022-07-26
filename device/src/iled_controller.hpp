#pragma once

#include "led_device.h"

struct ILEDController
{
    virtual LEDState GetState() = 0;
    virtual void SetState(const LEDState& state) = 0;
};
