#pragma once

#include <future>

#include "../../common/led_device.h"

struct LEDDevice : winrt::implements<LEDDevice, winrt::Windows::Foundation::IClosable>
{
	//! User facing LED state
	struct State {
		bool on;		//!< All leds on/off
		float warm;		//!< Warm component percentage 0-1
		float cool;		//!< Cool component percentage 0-1

		State(::LEDs::Common::LEDState& usb_state);
		explicit State(bool on, float warm, float cool);
	};

	//! \param dispatcher pass the main window dispatcher
	explicit LEDDevice(winrt::Microsoft::UI::Dispatching::DispatcherQueue dispatcher);
	~LEDDevice();

	//! Event handler type for device connected/disconnected
	using OnLEDConnected = winrt::delegate<void(bool)>;

	//! Event handler type for state changed
	using OnLEDStateChanged = winrt::delegate<void(State)>;

	void OnConnected(OnLEDConnected handler);
	void OnStateChanged(OnLEDStateChanged handler);

	void DiscoverDevice();
	void Close();

	winrt::Windows::Foundation::IAsyncOperation<bool> SetLEDs(bool on, float warm, float cool);
	winrt::Windows::Foundation::IAsyncOperation<bool> SetLEDs(float warm, float cool);
	winrt::Windows::Foundation::IAsyncOperation<bool> SetLEDs(const ::LEDs::Common::LEDState& state);
	winrt::Windows::Foundation::IAsyncOperation<bool> RequestLEDs();
	winrt::Windows::Foundation::IAsyncOperation<bool> SetOn(bool on);

private:
	winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcher;
	winrt::Windows::Devices::Enumeration::DeviceWatcher m_watcher{ nullptr };
	winrt::Windows::Devices::HumanInterfaceDevice::HidDevice m_device{ nullptr };
	winrt::hstring m_device_id {};
	
	OnLEDConnected m_connected_handler {};
	OnLEDStateChanged m_changed_handler {};

	::LEDs::Common::LEDState m_last{};
	using OpPromise = std::promise<bool>;
	std::weak_ptr<OpPromise> m_op_promise{};

	void StopWatcher();

	winrt::fire_and_forget OnDeviceAdded(
		winrt::Windows::Devices::Enumeration::DeviceWatcher sender,
		winrt::Windows::Devices::Enumeration::DeviceInformation deviceInterface);

	winrt::fire_and_forget OnDeviceRemoved(
		winrt::Windows::Devices::Enumeration::DeviceWatcher sender,
		winrt::Windows::Devices::Enumeration::DeviceInformationUpdate deviceUpdate);

	winrt::fire_and_forget OnInputReportReceived(
		winrt::Windows::Devices::HumanInterfaceDevice::HidDevice,
		winrt::Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs);

	/*!
	 * Send an operation to the deviceand wait a response with a timeout
	 * \return true and the response on success, false if the device is busy. Throws on any other condition.
	 */
	winrt::Windows::Foundation::IAsyncOperation<bool> SendOp(
		::LEDs::Common::USBMessageTypes msg, const ::LEDs::Common::LEDState& state);

	//! Send a report to the device and wait for the bytes to be written
	winrt::Windows::Foundation::IAsyncOperation<bool> SendReport(
		::LEDs::Common::USBMessageTypes msg, const ::LEDs::Common::LEDState& state);

	//! Parse a USB input report into it's components, throw on failure
	void ParseInputReport(
		const winrt::Windows::Devices::HumanInterfaceDevice::HidInputReport& report,
		::LEDs::Common::USBMessageTypes& msg,
		::LEDs::Common::LEDState& state);
};
