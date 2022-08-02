#pragma once

#include <functional>

#include "../../common/led_device.h"

struct LEDDevice
{
	using OnLEDStateChange = std::function<void(bool, float, float)>;

	LEDDevice(OnLEDStateChange handler);
	~LEDDevice();

	winrt::Windows::Foundation::IAsyncAction DiscoverDevice(bool refresh_state = true);

	void SetLEDs(bool on, float warm, float cool);
	void RequestLEDs();

private:
	winrt::Windows::Devices::HumanInterfaceDevice::HidDevice m_device{ nullptr };
	OnLEDStateChange m_handler {};

	void OnInputReportRecieved(
		winrt::Windows::Devices::HumanInterfaceDevice::HidDevice,
		winrt::Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs);

	winrt::fire_and_forget SendReport(USBMessageTypes msg, bool on, float warm, float cool);
};

