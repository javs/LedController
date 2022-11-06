#include <limits>

#include "led_component.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

using namespace LEDs::Common;

const microseconds LEDComponent::Period = 5ms; // 200hz

LEDComponent::LEDComponent(PinName pin)
 : m_Pin(pin)
{
    m_Pin.period_us(Period.count());
}

RawLEDComponentType LEDComponent::Get()
{
    return m_Pin * std::numeric_limits<RawLEDComponentType>::max();
}

void LEDComponent::Set(RawLEDComponentType raw)
{
    m_Pin = static_cast<float>(raw) / std::numeric_limits<RawLEDComponentType>::max();
}

float LEDComponent::GetPercentage()
{
    return m_Pin * 100.0f;
}
