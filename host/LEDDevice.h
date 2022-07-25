#pragma once

#include <winrt/Windows.Devices.HumanInterfaceDevice.h>

class LEDDevice
{
	winrt::Windows::Devices::HumanInterfaceDevice::HidDevice m_device{nullptr};

public:
	LEDDevice() = default;
	winrt::fire_and_forget DiscoverDevice();
};

