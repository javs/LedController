#pragma once

#include <cstdint>

const uint8_t DeviceVersionMajor = 1; //! Major version of the device code
const uint8_t DeviceVersionMinor = 0; //! Minor version of the device code

using RawLEDComponentType = uint16_t;

#pragma pack(push, 1)
/** The configurable state of the LEDs
 * \note: packed for transmission via USB.
 */
struct LEDState {
    bool                on;     //!< LEDs ON/OFF
    RawLEDComponentType warm;   //!< 0: warm component off, max: warm component at full brightness
    RawLEDComponentType cool;   //!< 0: cool component off, max: cool component at full brightness

    bool operator==(const LEDState& rhs) const = default;
};
#pragma pack(pop)


enum class USBMessageTypes : uint8_t {
    GetLEDState,
    SetLEDState,
};

//! See https://github.com/obdev/v-usb/blob/master/usbdrv/USB-IDs-for-free.txt
const uint16_t       USBVendorId             = 0x16C0;  //!< Van Ooijen Technische Informatica
const uint16_t       USBProductId            = 0x05DF;  //!< Generic HID
const uint16_t       USBProductRelease       = 0x0001;
const uint16_t       USBUsagePage            = 0xFFAB;  //!< Vendor specific. Only used in host, hardcoded by mbed.
const uint16_t       USBUsageId              = 0x0200;  //!< Vendor specific. Only used in host, hardcoded by mbed.
constexpr uint8_t    USBReportOutputLength   = sizeof(USBMessageTypes) + sizeof(LEDState);
constexpr uint8_t    USBReportInputLength    = sizeof(USBMessageTypes) + sizeof(LEDState);