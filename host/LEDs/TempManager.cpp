#include "pch.h"

#include "TempManager.h"

#include <winrt/Microsoft.Windows.System.h>

using namespace winrt;
using namespace winrt::Microsoft::UI::Dispatching;
using namespace std::literals::chrono_literals;

TempManager::TempManager()
	: m_curve{LEDCurve::Standard()}
{
	m_timer.Interval(30s);
	m_timer.Tick([this](auto&, auto&) {this->Update();});
}

void TempManager::SetDevice(winrt::com_ptr<LEDDevice>& device)
{
	m_device = device;
}

fire_and_forget TempManager::Update()
{
	if (!IsEnabled())
		co_return;

	auto led_step = m_curve.GetLEDAtCurrentTime();
	
	if (auto device{m_device.get()})
		co_await device->SetLEDs(led_step.warm / 100.0f, led_step.cold / 100.0f);
}

void TempManager::Enable(bool enable)
{
	if (IsEnabled() == enable)
		return;

	if (enable)
	{
		m_timer.Start();
		Update();
	}
	else
		m_timer.Stop();
}

bool TempManager::IsEnabled()
{
	return m_timer.IsEnabled();
}