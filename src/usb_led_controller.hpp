#include <functional>
#include <usb/USBHID.h>


/*!
 * Exposes a USB HID device to the host.
 * \note For STM32F, uses PA_11 USB_DM / PA_12 USB_DP
 */
class USBLEDController : public USBHID
{
public:
    using LEDState = std::pair<uint16_t, uint16_t>;

    /**
     * \param get_handler a handler that must return back the current state.
     * \param set_handler a handler that must set the state passed as argument.
     */
    USBLEDController(std::function<LEDState()>&& get_handler,
        std::function<void(const LEDState&)>&& set_handler);
    
    ~USBLEDController();

    //! Send a usb message with the indicated state.
    void SendLEDState(LEDState state);

private:
    enum class MessageTypes : uint8_t {
        GetLEDState,
        SetLEDState,
    };

    //! See https://github.com/obdev/v-usb/blob/master/usbdrv/USB-IDs-for-free.txt
    static const uint16_t VendorId              = 0x16c0;
    static const uint16_t ProductId             = 0x05DF;
    static const uint16_t ProductRelease        = 0x1;
    static constexpr uint8_t ReportOutputLength = sizeof(MessageTypes) + sizeof(LEDState);
    static constexpr uint8_t ReportInputLength  = sizeof(MessageTypes) + sizeof(LEDState);

    std::function<LEDState()> m_GetStateHandler;
    std::function<void(const LEDState&)> m_SetStateHandler;

    void report_rx() override;
};
