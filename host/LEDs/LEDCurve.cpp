#include "pch.h"

#include <cmath>

#include "LEDCurve.h"


using namespace std;
using namespace std::chrono;

LEDCurve LEDCurve::Standard()
{
	LEDCurve c{};

	c.AddStep(0, 0, LEDCurve::LEDControl{
		.warm = 50,
		.cold = 10,
		});

	c.AddStep(5, 0, LEDCurve::LEDControl{
		.warm = 50,
		.cold = 40,
		});

	c.AddStep(10, 0, LEDCurve::LEDControl{
		.warm = 40,
		.cold = 60,
		});

	c.AddStep(11, 0, LEDCurve::LEDControl{
		.warm = 45,
		.cold = 65,
		});

	c.AddStep(18, 0, LEDCurve::LEDControl{
		.warm = 45,
		.cold = 65,
		});

	c.AddStep(23, 30, LEDCurve::LEDControl{
		.warm = 50,
		.cold = 10,
		});

	return c;
}


void LEDCurve::AddStep(CurveStep step)
{
	m_steps.emplace(move(step));
}

void LEDCurve::AddStep(uint8_t day_hs, uint8_t day_mins, LEDControl led)
{
	hours time { day_hs };
	time += duration_cast<hours>(minutes(day_mins));

	m_steps.emplace(CurveStep{
		.time = time,
		.led = led,
		});
}


LEDCurve::LEDControl LEDCurve::GetLEDAtCurrentTime() const
{
	const auto now = zoned_time{ current_zone(), system_clock::now() }.get_local_time();
	return GetLED(now);
}

LEDCurve::LEDControl LEDCurve::GetLED(std::chrono::local_time<std::chrono::system_clock::duration> t) const
{
	const auto t_days = floor<days>(t);
	const auto time_of_day = floor<minutes>(t - t_days);

	if (m_steps.empty())
		return {};

	auto step = m_steps.begin();
	
	// Within the first step
	if (time_of_day <= step->time)
		return step->led;

	auto prev_step = step++;

	// Search segment to do lerp on
    for (; step != m_steps.end(); prev_step = step++)
		if (time_of_day <= step->time)
			return Lerp(*prev_step, *step, time_of_day);

	// Return the last step
	return prev_step->led;
}

LEDCurve::LEDControl LEDCurve::Lerp(const CurveStep& a, const CurveStep& b, const TimeOfDay& time) const
{
	const auto t = static_cast<float>(time.count() - a.time.count()) / (b.time.count() - a.time.count());
	
	return {
		.warm = lerp(a.led.warm, b.led.warm, static_cast<float>(t)),
		.cold = lerp(a.led.cold, b.led.cold, static_cast<float>(t)),
	};
}

bool LEDCurve::CurveStep::TimeLessThan(const CurveStep& lhs, const CurveStep& rhs)
{
	return lhs.time < rhs.time;
}
