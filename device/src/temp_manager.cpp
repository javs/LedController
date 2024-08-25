#include "temp_manager.hpp"

#include <mbed.h>

#include <chrono>
#include <events/mbed_events.h>

#include "apds9960_driver.hpp"

using namespace std::chrono_literals;
using namespace std::chrono;


TempManager::TempManager(ILEDController& controller, APDS9960Driver& light_sensor)
    : m_light_sensor(light_sensor)
    , m_controller(controller)
{
    auto queue = mbed::mbed_event_queue();

    m_timer = queue->call_every(3s, this, &TempManager::Update);
}

TempManager::~TempManager()
{
    if (m_timer)
        mbed::mbed_event_queue()->cancel(m_timer);
}

void TempManager::Update()
{
    APDS9960Driver::ColorSample color;
    m_light_sensor.GetColorData(color);

    printf("Ambient light: 0x%x\n", color.clear);
}
