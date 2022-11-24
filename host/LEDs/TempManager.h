#pragma once

#include "LEDDevice.h"
#include "LEDCurve.h"

class TempManager
{
	// Disable copying and moving, as m_timer isnt.
	TempManager(const TempManager&) = delete;
	TempManager& operator=(const TempManager&) = delete;
	TempManager& operator=(TempManager&& other) noexcept = delete;
	TempManager(TempManager&&) noexcept = delete;

	winrt::weak_ref<LEDDevice> m_device {};
	winrt::Microsoft::UI::Xaml::DispatcherTimer m_timer {};
	LEDCurve m_curve;

public:
	TempManager();
	
	//! Refresh current LEDs state
	winrt::Windows::Foundation::IAsyncOperation<bool> Update();

	void SetDevice(winrt::com_ptr<LEDDevice>& device);

	//! Whether this object acts or not. Starts disabled.
	void Enable(bool enable);

	//! \return whether the manager is enabled or not.
	bool IsEnabled();
};
