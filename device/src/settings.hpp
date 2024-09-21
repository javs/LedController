#pragma once

#include <memory>
#include <limits>

#include <FlashIAP.h>
#include <FlashIAP/FlashIAPBlockDevice.h>
#include <tdbstore/TDBStore.h>

#include "led_device.h"

//! Persistent settings
class Settings {
    mbed::FlashIAP flash_all;
    std::unique_ptr<FlashIAPBlockDevice> flash_data;
    std::unique_ptr<mbed::TDBStore> key_store;

    bool                                m_SettingOn     = true;
    LEDs::Common::RawLEDComponentType   m_SettingWarm   =
        std::numeric_limits<LEDs::Common::RawLEDComponentType>::max() / 2;
    LEDs::Common::RawLEDComponentType   m_SettingCool   = 0;
    uint16_t                            m_SettingLSMin  = 0x500;
    uint16_t                            m_SettingLSMax  = 0x6100;

    //! Read all settings and write defaults if not found
    void ReadSettings();

    Settings();

public:
    static Settings& get();
    
    void PrintDiags() const;

    bool GetOn() const;
    void SetOn(bool on);

    LEDs::Common::RawLEDComponentType GetWarm() const;
    void SetWarm(LEDs::Common::RawLEDComponentType warm);

    LEDs::Common::RawLEDComponentType GetCool() const;
    void SetCool(LEDs::Common::RawLEDComponentType cool);

    uint16_t GetLightSensorMin() const;
    void SetLightSensorMin(uint16_t val);

    uint16_t GetLightSensorMax() const;
    void SetLightSensorMax(uint16_t val);
};
