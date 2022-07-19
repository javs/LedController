#include <USBHID.h>

//! Exposes a USB HID device to the host.
class USBLEDController : public USBHID
{
    //! See https://github.com/obdev/v-usb/blob/master/usbdrv/USB-IDs-for-free.txt
    static const uint16_t VendorId          = 0x16c0;
    static const uint16_t ProductId         = 0x05DF;
    static const uint16_t ProductRelease    = 0x1;
    static const uint8_t ReportOutputLength = 8;
    static const uint8_t ReportInputLength  = 8;

    void report_rx() override;

public:
    USBLEDController();
    ~USBLEDController();
};
