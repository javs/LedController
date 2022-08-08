#pragma once

#include <set>
#include <chrono>

struct LEDCurve
{
	struct LEDControl {
		float warm;
		float cold;

		bool operator==(const LEDControl& rhs) const = default;
	};

	using TimeOfDay = std::chrono::minutes;

	struct CurveStep {
		TimeOfDay time;
		LEDControl led;

		static bool TimeLessThan(const CurveStep& lhs, const CurveStep& rhs);
	};

	//! Add a curve time vs. LED state step.
	void AddStep(CurveStep step);

	//! Add a curve time (in hours and minutes) vs. LED state step.
	void AddStep(uint8_t day_hs, uint8_t day_mins, LEDControl led);

	//! \return LED state at current time.
	LEDControl GetLEDAtCurrentTime() const;

	LEDControl GetLED(std::chrono::local_time<std::chrono::system_clock::duration> t) const;

private:
	LEDControl Lerp(const CurveStep& a, const CurveStep& b, const TimeOfDay& time) const;

	std::set<CurveStep, decltype(&CurveStep::TimeLessThan)> m_steps{&CurveStep::TimeLessThan};
};

