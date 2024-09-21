#pragma once

#include "fir_filter.hpp"
#include "led_curve.hpp"
#include "led_controller.hpp"

class APDS9960Driver;

/**
 * A ILEDController that adds:
 * - Sensor adjusted brightness
 * - Time of day adjusted warmth
 */
class AutoLEDController : public LEDController {
    int m_Timer {};
    APDS9960Driver& m_LightSensor;
    LEDCurve m_Curve;
    bool m_AutoLevels {true};
    FIRFilter<10> m_FilteredLightSensor;
    
    void AutoUpdate();
    void SetupTimer();

    float GetSensorBrightnessAdjustment();
    uint16_t GetLightSensorRawValue();

public:
    explicit AutoLEDController(PinName cool_pin, PinName warm_pin, PinName user_button_pin,
        APDS9960Driver& light_sensor);
    AutoLEDController(const AutoLEDController&) = delete;
    ~AutoLEDController();

    LEDs::Common::LEDState GetState() const override;
    void SetState(LEDs::Common::LEDState&) override;

    void SetLightSensorRange(const LEDs::Common::LightSensorRange& range) override;
};
