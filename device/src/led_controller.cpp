
#include <mbed.h>
#include <events/mbed_events.h>

#include "led_controller.hpp"
#include "settings.hpp"


using namespace std::chrono_literals;
using namespace LEDs::Common;

const std::chrono::milliseconds LEDController::HighPowerTimeLimit   = 5min;


LEDController::LEDController(PinName cool_pin, PinName warm_pin, PinName user_button_pin)
    : m_Cool(cool_pin)
    , m_Warm(warm_pin)
    , m_Button(user_button_pin)
{
    m_Button.rise([this](){
        auto queue = mbed::mbed_event_queue();

        queue->call(this, &LEDController::ToggleOn);
    });

    // Don't begin the initial transition on the main code start path to avoid stutters
    auto queue = mbed::mbed_event_queue();

    queue->call(this, &LEDController::UpdateLEDs, true);
}

void LEDController::ToggleOn()
{
    auto& settings = Settings::get();

    this->m_User = true;
    settings.SetOn(!settings.GetOn());
    this->UpdateLEDs(true);
}

void LEDController::UpdateLEDs(bool notify)
{
    auto& settings = Settings::get();

    auto on = settings.GetOn();
    m_Cool.Set(on ? settings.GetCool() : 0);
    m_Warm.Set(on ? settings.GetWarm() : 0);

    auto queue = mbed::mbed_event_queue();
    
    if (m_LimitsTimer != 0)
        queue->cancel(m_LimitsTimer);
    
    if (InHighPower())
        m_LimitsTimer = queue->call_in(HighPowerTimeLimit, this, &LEDController::EnsureLimits);
    
    if (notify)
        m_ChangedDelegate(GetState());
}

void LEDController::EnsureLimits()
{
    if (InHighPower())
    {
        // get the reduction each component needs, in raw values
        const auto DistanceToLimit =
            (m_Cool.GetPercentage() + m_Warm.GetPercentage()) - TotalPowerLimitPercentage;

        const int RawComponentReduction =
            ((DistanceToLimit / 2.0f) / 100) * std::numeric_limits<RawLEDComponentType>::max();
        
        int new_warm = m_Warm.Get() - RawComponentReduction;
        int new_cool = m_Cool.Get() - RawComponentReduction;

        // a few guardrails in case the power limit ever becomes too low
        if (new_cool < 0 && new_cool < 0)
        {
            new_cool = 0;
            new_warm = 0;
        }
        else if (new_warm < 0)
        {
            new_cool -= std::abs(new_warm);
            new_warm = 0;
        }
        else if (new_cool < 0)
        {
            new_warm -= std::abs(new_cool);
            new_cool = 0;
        }

        auto& settings = Settings::get();

        settings.SetWarm(new_warm);
        settings.SetCool(new_cool);
        UpdateLEDs(true);
    }
}

LEDState LEDController::GetState() const
{
    auto& settings = Settings::get();

    return LEDState {
        .on = settings.GetOn(),
        .user = m_User,
        .warm = settings.GetWarm(),
        .cool = settings.GetCool(),
    };
}

void LEDController::SetState(LEDState& state)
{
    auto& settings = Settings::get();

    m_User = state.user;

    settings.SetOn(state.on);
    settings.SetWarm(state.warm);
    settings.SetCool(state.cool);
    
    UpdateLEDs(false);
}

bool LEDController::InHighPower()
{
    return (m_Cool.GetPercentage() + m_Warm.GetPercentage()) > TotalPowerLimitPercentage;
}

void LEDController::SetEventDelegate(OnStateChanged& delegate)
{
    m_ChangedDelegate = delegate;
}
