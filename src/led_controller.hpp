#pragma once

#include <InterruptIn.h>

#include "led_component.hpp"
#include "iled_controller.hpp"


//! Drives a LED strip with warm and cool lights
class LEDController : public ILEDController {
    LEDComponent m_Cool {D3 /*PB_3*/};
    LEDComponent m_Warm {D4 /*PB_5*/};
    mbed::InterruptIn m_Button {BUTTON1};

    void UpdateLEDs();

public:
    LEDController();

    void ToggleOn();

    LEDState GetState() override;
    void SetState(const LEDState&) override;
};
