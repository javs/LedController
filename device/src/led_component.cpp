#include <limits>

#include <events/mbed_events.h>

#include "led_component.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

using namespace LEDs::Common;

const microseconds  LEDComponent::Period = 5ms; // 200hz

const float         LEDComponent::ChangePerTick = 0.002f;
const milliseconds  LEDComponent::ChangeDelay   = 20ms;
const float         LEDComponent::UpdateCutoff  = 0.0001f;

LEDComponent::LEDComponent(PinName pin)
 : m_Pin(pin)
{
    m_Pin.period_us(Period.count());
    m_SetPoint = m_Pin;
}

RawLEDComponentType LEDComponent::Get()
{
    return m_SetPoint * std::numeric_limits<RawLEDComponentType>::max();
}

void LEDComponent::Set(RawLEDComponentType raw)
{
    m_SetPoint = static_cast<float>(raw) / std::numeric_limits<RawLEDComponentType>::max();
    UpdatePin();
}

float LEDComponent::GetPercentage()
{
    return m_SetPoint * 100.0f;
}

void LEDComponent::UpdatePin()
{
    if (IsUpdating())
        m_Pin = m_Pin + std::clamp(m_SetPoint - m_Pin, -ChangePerTick, ChangePerTick);
    else
        m_Pin = m_SetPoint; // Set it anyway to try overcome any difference in precision

    CheckForUpdate();
}

bool LEDComponent::IsUpdating()
{
    return std::abs(m_SetPoint - m_Pin) > UpdateCutoff;
}

void LEDComponent::CheckForUpdate()
{
    auto queue = mbed::mbed_event_queue();

    if (IsUpdating())
    {
        if (m_UpdateTimer == 0)
            m_UpdateTimer = queue->call_every(ChangeDelay, this, &LEDComponent::UpdatePin);
    }
    else
    {
        if (m_UpdateTimer != 0)
        {
            // Ignore result, will return false due to cancelling within timer,
            // but will cancel anyway. 
            queue->cancel(m_UpdateTimer);
            m_UpdateTimer = 0;
        }
    }
}
