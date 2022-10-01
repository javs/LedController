#pragma once

#include "../../common/led_device.h"

struct LEDDevice
{
	explicit LEDDevice(winrt::Microsoft::UI::Dispatching::DispatcherQueue dispatcher);
	~LEDDevice();

	//! Event handler type for device connected/disconnected
	using OnLEDConnected = winrt::delegate<void(bool)>;

	//! Event handler type for state changed
	using OnLEDStateChanged = winrt::delegate<void(bool, float, float)>;

	void OnConnected(OnLEDConnected handler);
	void OnStateChanged(OnLEDStateChanged handler);

	void DiscoverDevice();
	void Close();

	winrt::Windows::Foundation::IAsyncAction SetLEDs(bool on, float warm, float cool);
	winrt::Windows::Foundation::IAsyncAction SetLEDs(const LEDState& state);
	winrt::Windows::Foundation::IAsyncOperation<bool> RequestLEDs();
	winrt::Windows::Foundation::IAsyncAction SetOn(bool on);

private:
	winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcher;
	winrt::Windows::Devices::Enumeration::DeviceWatcher m_watcher{ nullptr };
	winrt::Windows::Devices::HumanInterfaceDevice::HidDevice m_device{ nullptr };
	winrt::hstring m_device_id {};
	
	OnLEDConnected m_connected_handler {};
	OnLEDStateChanged m_changed_handler {};

	LEDState m_last{};

	void StopWatcher();

	winrt::fire_and_forget OnDeviceAdded(
		winrt::Windows::Devices::Enumeration::DeviceWatcher sender,
		winrt::Windows::Devices::Enumeration::DeviceInformation deviceInterface);

	winrt::fire_and_forget OnDeviceRemoved(
		winrt::Windows::Devices::Enumeration::DeviceWatcher sender,
		winrt::Windows::Devices::Enumeration::DeviceInformationUpdate deviceUpdate);

	winrt::fire_and_forget OnInputReportRecieved(
		winrt::Windows::Devices::HumanInterfaceDevice::HidDevice,
		winrt::Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs);

	winrt::Windows::Foundation::IAsyncOperation<bool> SendReport(USBMessageTypes msg, const LEDState& state);
};

