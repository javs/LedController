#include "settings.hpp"

#include <FlashIAP/FlashIAPBlockDevice.h>
#include <tdbstore/TDBStore.h>

using namespace LEDs::Common;

Settings::Settings()
{
    auto ret = flash_all.init();
    if (ret != MBED_SUCCESS)
        printf("Failed to init all flash: %i", ret);

    uint32_t flash_size = 30 * 1024;
    uint32_t flash_start =
        flash_all.get_flash_start() + (flash_all.get_flash_size() - flash_size);

    flash_data = std::make_unique<FlashIAPBlockDevice>(flash_start, flash_size);
    ret = flash_data->init();

    if (ret != MBED_SUCCESS)
        printf("Failed to init data flash: %i", ret);
    
    key_store = std::make_unique<mbed::TDBStore>(flash_data.get());

    ret = key_store->init();
    if (ret != MBED_SUCCESS)
        printf("Failed to init kv store: %i", ret);

    ReadSettings();
}

Settings& Settings::get()
{
    static Settings instance;
    return instance;
}

void Settings::PrintDiags()
{
    printf("-- Flash Diagnostics --\n");

    uint32_t flash_start = flash_all.get_flash_start();
    printf("start address: 0x%lx\n", flash_start);
    
    uint32_t flash_size = flash_all.get_flash_size();
    printf("flash size: 0x%lx\n", flash_size);
    
    uint32_t page_size = flash_all.get_page_size();
    printf("page size: %lu\n", page_size);
    
    uint32_t last_sector_size = flash_all.get_sector_size(flash_start + flash_size - 1);
    printf("last sector size: %lu\n", last_sector_size);

    printf("-- Flash Diagnostics --\n");
}

void Settings::ReadSettings()
{
    if (key_store->get("on", &m_SettingOn, sizeof(m_SettingOn)) != MBED_SUCCESS)
        SetOn(m_SettingOn);
    
    if (key_store->get("warm", &m_SettingWarm, sizeof(m_SettingWarm)) != MBED_SUCCESS)
        SetWarm(m_SettingWarm);

    if (key_store->get("cool", &m_SettingCool, sizeof(m_SettingCool)) != MBED_SUCCESS)
        SetCool(m_SettingCool);
}

bool Settings::GetOn()
{
    return m_SettingOn;
}

void Settings::SetOn(bool on)
{
    m_SettingOn = on;

    auto res = key_store->set("on", &on, sizeof(on), 0);

    if (res != MBED_SUCCESS)
        printf("Failed to write on setting: %i", res);
}

RawLEDComponentType Settings::GetWarm()
{
    return m_SettingWarm;
}

void Settings::SetWarm(RawLEDComponentType warm)
{
    m_SettingWarm = warm;
    
    auto res = key_store->set("warm", &warm, sizeof(warm), 0);

    if (res != MBED_SUCCESS)
        printf("Failed to write warm setting: %i", res);
}

RawLEDComponentType Settings::GetCool()
{
    return m_SettingCool;
}

void Settings::SetCool(RawLEDComponentType cool)
{
    m_SettingCool = cool;
    
    auto res = key_store->set("cool", &cool, sizeof(cool), 0);

    if (res != MBED_SUCCESS)
        printf("Failed to write cool setting: %i", res);
}
