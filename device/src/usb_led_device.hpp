#pragma once

#include <functional>
#include <usb/USBHID.h>

#include "iled_controller.hpp"

/*!
 * Exposes a USB HID device to the host.
 * \note For STM32F3, PA_11 USB_DM / PA_12 USB_DP.
 * \note Using USB Full Speed (D+ board pullup).
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
    void SendLEDState(LEDState state);

private:

    ILEDController& m_Controller;

    void report_rx() override;
};
