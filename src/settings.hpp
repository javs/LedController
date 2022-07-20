
#include <memory>
#include <limits>

#include <FlashIAP.h>
#include <FlashIAP/FlashIAPBlockDevice.h>
#include <tdbstore/TDBStore.h>

//! Persistent settings
class Settings {
    mbed::FlashIAP flash_all;
    std::unique_ptr<FlashIAPBlockDevice> flash_data;
    std::unique_ptr<mbed::TDBStore> key_store;

    bool        m_SettingOn     = true;
    uint16_t    m_SettingWarm   = std::numeric_limits<uint16_t>::max() / 2;
    uint16_t    m_SettingCool   = 0;

    //! Read all settings and write defaults if not found
    void ReadSettings();

    Settings();

public:
    static Settings& get();
    
    void PrintDiags();

    bool GetOn();
    void SetOn(bool on);

    uint16_t GetWarm();
    void SetWarm(uint16_t warm);

    uint16_t GetCool();
    void SetCool(uint16_t cool);
};
