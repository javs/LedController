#include <limits>

#include <events/mbed_events.h>

#include "led_component.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

using namespace LEDs::Common;

const microseconds  LEDComponent::Period = 5ms; // 200hz

const milliseconds  LEDComponent::ChangeTick    = 25ms;
const milliseconds  LEDComponent::ChangeTime    = 3000ms;
const float         LEDComponent::UpdateCutoff  = 0.00001f;

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
    m_SetPointEnd = rtos::Kernel::Clock::now() + ChangeTime;
    m_UpdateSpeed = (m_SetPoint - m_Pin) / (ChangeTime / ChangeTick);
    
    UpdatePin();
}

float LEDComponent::GetPercentage()
{
    return m_SetPoint * 100.0f;
}

void LEDComponent::UpdatePin()
{
    auto queue = mbed::mbed_event_queue();

    auto now = rtos::Kernel::Clock::now();

    // End of update, or difference too small
    if (now >= m_SetPointEnd || std::abs(m_SetPoint - m_Pin) < UpdateCutoff) {
        // Set it anyway to try overcome any difference in precision
        m_Pin = m_SetPoint;

        if (m_UpdateTimer != 0)
        {
            // Ignore result, will return false due to cancelling within timer,
            // but will cancel anyway. 
            queue->cancel(m_UpdateTimer);
            m_UpdateTimer = 0;
        }
    } else {
        m_Pin = m_Pin + m_UpdateSpeed;

        if (m_UpdateTimer == 0)
            m_UpdateTimer = queue->call_every(ChangeTick, this, &LEDComponent::UpdatePin);
    }
}
