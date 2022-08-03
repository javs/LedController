#pragma once

#include <functional>

#include "../../common/led_device.h"

struct LEDDevice
{
	using OnLEDStateChange = std::function<void(bool, float, float)>;

	LEDDevice(OnLEDStateChange handler);
	~LEDDevice();

	winrt::Windows::Foundation::IAsyncAction DiscoverDevice(bool refresh_state = true);

	winrt::Windows::Foundation::IAsyncAction SetLEDs(bool on, float warm, float cool);
	winrt::Windows::Foundation::IAsyncAction SetLEDs(const LEDState& state);
	winrt::Windows::Foundation::IAsyncAction RequestLEDs();
	winrt::Windows::Foundation::IAsyncAction SetOn(bool on);

private:
	winrt::Windows::Devices::HumanInterfaceDevice::HidDevice m_device{ nullptr };
	OnLEDStateChange m_handler {};
	LEDState m_last{};

	void OnInputReportRecieved(
		winrt::Windows::Devices::HumanInterfaceDevice::HidDevice,
		winrt::Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs);

	winrt::Windows::Foundation::IAsyncAction SendReport(USBMessageTypes msg, const LEDState& state);
};

