#pragma once

#include <ctime>
#include <cstdint>

//! Types common to both device and host
namespace LEDs::Common {

    const uint8_t DeviceVersionMajor = 2; //! Major version of the device code
    const uint8_t DeviceVersionMinor = 2; //! Minor version of the device code

    using RawLEDComponentType = uint16_t;

    #pragma pack(push, 1)
    /** The configurable state of the LEDs
     * \note: packed for transmission via USB.
     */
    struct LEDState {
        time_t              current_time;   //!< Current wall localtime clock
        bool                on;             //!< LEDs ON/OFF
        bool                user;           //!< On/Off is currently overriden by a user
        bool                auto_levels;    //!< Warmth and Brightness is controlled by device
        RawLEDComponentType warm;           //!< 0: warm component off, max: warm component at full brightness
        RawLEDComponentType cool;           //!< 0: cool component off, max: cool component at full brightness
    };

    //! Calibration range for light sensor adjustment of brightness.
    struct LightSensorRange {
        uint16_t            max;            //!< Sensor reading for minimum brightness.
        uint16_t            min;            //!< Sensor reading for maximum brightness.
    };
    #pragma pack(pop)

    //! Message IDs sent/received via usb, from the device perspective.
    enum class USBMessageTypes : uint8_t {
        GetLEDState,        //!< Sent/Received to get state. Received when power limit enforced or blue button pressed.
        SetLEDState,        //!< Sent/Received to set state. Response confirms state.
        SetLightSensorRange,//!< Sent/Received to change light sensor range. Response confirms state.
    };

    //! See https://github.com/obdev/v-usb/blob/master/usbdrv/USB-IDs-for-free.txt
    const uint16_t       USBVendorId             = 0x16C0;  //!< Van Ooijen Technische Informatica
    const uint16_t       USBProductId            = 0x05DF;  //!< Generic HID
    const uint16_t       USBProductRelease       = 0x0001;
    const uint16_t       USBUsagePage            = 0xFFAB;  //!< Vendor specific. Only used in host, hardcoded by mbed.
    const uint16_t       USBUsageId              = 0x0200;  //!< Vendor specific. Only used in host, hardcoded by mbed.

    // mbed only supports one input and one output report, so
    // pick the max size among all variants for each.
    constexpr uint8_t    USBReportOutputLength   = sizeof(USBMessageTypes) + sizeof(LEDState);
    constexpr uint8_t    USBReportInputLength    = sizeof(USBMessageTypes) + sizeof(LEDState);

    //! The max amount combined components can go, in percentages.
    //! Device allows higher than this (up to 200%), but only for a short time.
    const float          TotalPowerLimitPercentage = 110.0f;
}
