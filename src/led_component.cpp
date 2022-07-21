#include <limits>

#include "led_component.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

const microseconds LEDComponent::Period = 5ms;

LEDComponent::LEDComponent(PinName pin)
 : m_Pin(pin)
{
    m_Pin.period_us(Period.count());
}

uint16_t LEDComponent::Get()
{
    return m_Pin * std::numeric_limits<uint16_t>::max();
}

void LEDComponent::Set(uint16_t raw)
{
    m_Pin = static_cast<float>(raw) / std::numeric_limits<uint16_t>::max();
}
