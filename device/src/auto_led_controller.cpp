#include "auto_led_controller.hpp"

#include <mbed.h>

#include <chrono>
#include <events/mbed_events.h>

#include "apds9960_driver.hpp"
#include "iled_controller.hpp"

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace LEDs::Common;


AutoLEDController::AutoLEDController(PinName cool_pin, PinName warm_pin,
    PinName user_button_pin, APDS9960Driver& light_sensor)
    : LEDController(cool_pin, warm_pin, user_button_pin)
    , m_LightSensor(light_sensor)
    , m_Curve(LEDCurve::Standard())
{
    AutoUpdate();
}

AutoLEDController::~AutoLEDController()
{
    if (m_Timer)
        mbed::mbed_event_queue()->cancel(m_Timer);
}

void AutoLEDController::AutoUpdate()
{
    auto state = GetState();

    if (state.auto_levels)
        SetState(state);
}

LEDState AutoLEDController::GetState() const
{
    auto state = LEDController::GetState();
    state.current_time = time(nullptr);
    state.auto_levels = m_AutoLevels;

    return state;
}

void AutoLEDController::SetState(LEDState& state)
{
    m_AutoLevels = state.auto_levels;

    if (difftime(time(nullptr), state.current_time) > 5)
        set_time(state.current_time);

    if (m_AutoLevels)
    {
        auto curve_state = m_Curve.GetLEDAtCurrentTime();
        state.warm = curve_state.warm * std::numeric_limits<RawLEDComponentType>::max();
        state.cool = curve_state.cold * std::numeric_limits<RawLEDComponentType>::max();
        
        APDS9960Driver::ColorSample color;
        m_LightSensor.GetColorData(color);

        printf("Ambient light: 0x%x\n", color.clear);
        // TODO: find min/max and apply
    }

    LEDController::SetState(state);

    SetupTimer();
}

void AutoLEDController::SetupTimer()
{
    auto queue = mbed::mbed_event_queue();

    if (m_AutoLevels)
    {
        if (!m_Timer)
            m_Timer = queue->call_every(5s, this, &AutoLEDController::AutoUpdate);
    } else {
        if (m_Timer)
        {
            queue->cancel(m_Timer);
            m_Timer = 0;
        }
    }
}
