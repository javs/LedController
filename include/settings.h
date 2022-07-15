
#include <memory>

#include <FlashIAP.h>
#include <FlashIAPBlockDevice.h>
#include <TDBStore.h>

//! Persistent settings
class Settings {
    mbed::FlashIAP flash_all;
    std::unique_ptr<FlashIAPBlockDevice> flash_data;
    std::unique_ptr<mbed::TDBStore> key_store;

public:
    Settings();
    void PrintDiags();

    bool GetOn();
    void SetOn(bool on);
};
