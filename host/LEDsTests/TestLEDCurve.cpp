#include "pch.h"

#include "LEDCurve.h"

using namespace std;
using namespace std::chrono;

class LEDCurveTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		curve0.AddStep(0, 0, LEDCurve::LEDControl{
			.warm = 50,
			.cold = 40,
			});

		curve0.AddStep(10, 0, LEDCurve::LEDControl{
			.warm = 40,
			.cold = 60,
			});

		curve0.AddStep(11, 0, LEDCurve::LEDControl{
			.warm = 50,
			.cold = 70,
			});

		curve0.AddStep(18, 0, LEDCurve::LEDControl{
			.warm = 50,
			.cold = 70,
			});

		curve0.AddStep(23, 30, LEDCurve::LEDControl{
			.warm = 50,
			.cold = 10,
			});
	}

	LEDCurve curve0;		//!< Natural curve
};

TEST_F(LEDCurveTest, GetCurrentTimeStep)
{
	EXPECT_EQ(
		curve0.GetLED(local_days(2020y/March/1)),
		(LEDCurve::LEDControl{
			.warm = 50,
			.cold = 40,
		})) << "First edge";

	EXPECT_EQ(
		curve0.GetLED(local_days(2020y/October/5) + 23h + 59min),
		(LEDCurve::LEDControl{
			.warm = 50,
			.cold = 10,
			})) << "Last edge";

	EXPECT_EQ(
		curve0.GetLED(local_days(2020y/March/1) + 11h),
		(LEDCurve::LEDControl{
			.warm = 50,
			.cold = 70,
			})) << "Within a step";

	EXPECT_EQ(
		curve0.GetLED(local_days(2020y/March/1) + 5h),
		(LEDCurve::LEDControl{
			.warm = 45,
			.cold = 50,
			})) << "Half within a step";
}
