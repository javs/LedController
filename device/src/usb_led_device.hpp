#pragma once

#include <functional>
#include <usb/USBHID.h>

#include "led_device.h"
#include "iled_controller.hpp"

/*!
 * Exposes a USB HID device to the host.
 * \note For STM32F3, PA_11 USB_DM / PA_12 USB_DP.
 * \note Using USB Full Speed (D+ board pullup).
 */
class USBLEDDevice : public USBHID
{
public:
    USBLEDDevice(ILEDController& controller);
    
    ~USBLEDDevice();

    //! Send a usb message with the indicated id and state.
    template<typename TBody>
    void SendUSBMessage(LEDs::Common::USBMessageTypes id, const TBody& state);

private:

    ILEDController& m_Controller;

    void report_rx() override;

    void OnControllerStateChanged(const LEDs::Common::LEDState& state);
};
