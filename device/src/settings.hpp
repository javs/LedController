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

    bool                m_SettingOn     = true;
    RawLEDComponentType m_SettingWarm   = std::numeric_limits<RawLEDComponentType>::max() / 2;
    RawLEDComponentType m_SettingCool   = 0;

    //! Read all settings and write defaults if not found
    void ReadSettings();

    Settings();

public:
    static Settings& get();
    
    void PrintDiags();

    bool GetOn();
    void SetOn(bool on);

    RawLEDComponentType GetWarm();
    void SetWarm(RawLEDComponentType warm);

    RawLEDComponentType GetCool();
    void SetCool(RawLEDComponentType cool);
};
