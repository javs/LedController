#pragma once

#include "LEDCurve.h"

class TempManager
{
	// Disable copying and moving, as m_timer can't be.
	TempManager(const TempManager&) = delete;
	TempManager& operator=(const TempManager&) = delete;
	TempManager& operator=(TempManager&& other) noexcept = delete;
	TempManager(TempManager&&) noexcept = delete;

	using UpdateDelegate = winrt::delegate<void(float, float)>;
	UpdateDelegate m_delegate;
	winrt::Microsoft::UI::Xaml::DispatcherTimer m_timer {};
	LEDCurve m_curve;

public:
	TempManager();

	void OnUpdated(UpdateDelegate delegate);
	
	//! Refresh current LEDs state
	void Update();

	//! Whether this object acts or not. Starts disabled.
	void Enable(bool enable);

	//! \return whether the manager is enabled or not.
	bool IsEnabled();
};
