#include "settings.hpp"

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

bool Settings::GetOn()
{
    bool state = false;

    auto res = key_store->get("on", &state, sizeof(state));
    
    // Set default
    if (res != MBED_SUCCESS)
        SetOn(state);

    return state;
}

void Settings::SetOn(bool on)
{
    auto res = key_store->set("on", &on, sizeof(on), 0);

    if (res != MBED_SUCCESS)
        printf("Failed to write: %i", res);
}