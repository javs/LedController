#include <events/mbed_events.h>

#include "led_controller.hpp"
#include "settings.hpp"

#include <type_traits>

LEDController::LEDController()
{
    m_Button.rise([this](){
        auto queue = mbed::mbed_event_queue();

        queue->call(this, &LEDController::ToggleOn);
    });

    UpdateLEDs();
}

void LEDController::ToggleOn()
{
    auto& settings = Settings::get();

    settings.SetOn(!settings.GetOn());
    this->UpdateLEDs();
}

void LEDController::UpdateLEDs()
{
    auto& settings = Settings::get();

    auto on = settings.GetOn();
    m_Cool.Set(on ? settings.GetCool() : 0);
    m_Warm.Set(on ? settings.GetWarm() : 0);
}

ILEDController::LEDState LEDController::GetState()
{
    auto& settings = Settings::get();

    return ILEDController::LEDState {
        .on = settings.GetOn(),
        .warm = settings.GetWarm(),
        .cool = settings.GetCool(),
    };
}

void LEDController::SetState(const ILEDController::LEDState& state)
{
    auto& settings = Settings::get();

    settings.SetOn(state.on);
    settings.SetWarm(state.warm);
    settings.SetCool(state.cool);
    
    UpdateLEDs();
}
