#pragma once

#include "led_curve.hpp"

class APDS9960Driver;
class ILEDController;

class TempManager {
    int m_timer {};
    APDS9960Driver& m_light_sensor;
    ILEDController& m_controller;
    LEDCurve m_curve;
    
    void Update();

public:
    explicit TempManager(ILEDController& controller, APDS9960Driver& light_sensor);
    TempManager(const TempManager&) = delete;
    ~TempManager();
};
