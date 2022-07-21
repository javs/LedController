#pragma once

#include <functional>
#include <usb/USBHID.h>

#include "iled_controller.hpp"

/*!
 * Exposes a USB HID device to the host.
 * \note For STM32F3, uses PA_11 USB_DM / PA_12 USB_DP
 */
class USBLEDDevice : public USBHID
{
public:
    /**
     * \param get_handler a handler that must return back the current state.
     * \param set_handler a handler that must set the state passed as argument.
     */
    USBLEDDevice(ILEDController& controller);
    
    ~USBLEDDevice();

    //! Send a usb message with the indicated state.
    void SendLEDState(ILEDController::LEDState state);

private:
    enum class MessageTypes : uint8_t {
        GetLEDState,
        SetLEDState,
    };

    //! See https://github.com/obdev/v-usb/blob/master/usbdrv/USB-IDs-for-free.txt
    static const uint16_t VendorId              = 0x16c0;
    static const uint16_t ProductId             = 0x05DF;
    static const uint16_t ProductRelease        = 0x1;
    static constexpr uint8_t ReportOutputLength =
        sizeof(MessageTypes) + sizeof(ILEDController::LEDState);
    static constexpr uint8_t ReportInputLength  =
        sizeof(MessageTypes) + sizeof(ILEDController::LEDState);

    ILEDController& m_Controller;

    void report_rx() override;
};
