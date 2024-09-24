#pragma once

#include <future>

#include "../../common/led_device.h"

struct LEDDevice : winrt::implements<LEDDevice, winrt::Windows::Foundation::IClosable>
{
	//! User facing LED state
	struct State {
		bool on;			//!< All leds on/off
		bool user;			//!< On/Off is forced by user
		bool auto_levels;	//!< Device levels in auto mode
		float warm;			//!< Warm component percentage 0-1
		float cool;			//!< Cool component percentage 0-1

		static State FromUSB(const ::LEDs::Common::LEDState& usb_state);
		::LEDs::Common::LEDState ToUSB() const;
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

	winrt::Windows::Foundation::IAsyncOperation<bool> RequestLEDs();
	winrt::Windows::Foundation::IAsyncOperation<bool> SetIdle(bool idle);
	winrt::Windows::Foundation::IAsyncOperation<bool> SetLightSensorRange(uint16_t min, uint16_t max);
	winrt::Windows::Foundation::IAsyncOperation<bool> SetLEDs(State& state);

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

	winrt::Windows::Foundation::IAsyncOperation<bool> SetLEDs(::LEDs::Common::LEDState& state);

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
	template<typename TData>
	winrt::Windows::Foundation::IAsyncOperation<bool> SendOp(
		::LEDs::Common::USBMessageTypes msg, const TData& state);

	//! Send a report to the device and wait for the bytes to be written
	template<typename TData>
	winrt::Windows::Foundation::IAsyncOperation<bool> SendReport(
		::LEDs::Common::USBMessageTypes msg, const TData& state);

	//! Parse a USB input report into it's components, throw on failure
	void ParseInputReport(
		const winrt::Windows::Devices::HumanInterfaceDevice::HidInputReport& report,
		::LEDs::Common::USBMessageTypes& msg,
		::LEDs::Common::LEDState& state);

	//! Get time_t adjusted to account for the current local time, since the device has no timezones.
	time_t GetTimeForDevice() const;
};
