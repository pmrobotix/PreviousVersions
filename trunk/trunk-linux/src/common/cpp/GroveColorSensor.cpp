/*!
 * \file
 * \brief Implémentation de la classe GroveColorSensor concernant l'utilisation du détecteur Grove TSC3414.
 */

#include <math.h>
#include "GroveColorSensor.hpp"
#include "../../common/cpp/HostI2cBus.hpp"

pmx::GroveColorSensor::GroveColorSensor(pmx::Robot & robot)
		: ARobotElement(robot), connected_(false), integrationtime_(12), loopdelay_(12), percentageEnabled_(false), compensateEnabled_(
				false)
{
	try
	{
		utils::HostI2cBus::instance().open(); //TODO close it by the robot destructor
	} catch (utils::Exception * e)
	{
		logger().error() << "Exception open: " << e->what() << utils::end;
	}

	try
	{
		utils::HostI2cBus::instance().setSlave(GROVE_COLOR_DEFAULT_ADDRESS);
		connected_ = true;
	} catch (utils::Exception * e)
	{
		//deactivate the device if not connected
		connected_ = false;
		logger().error() << "Exception setSlave: " << e->what() << utils::end;
	}

	TCS3414Initialize(10, 100);
}

/*** Gets the blue sensor value and returns an unsigned int ***/
uint8_t pmx::GroveColorSensor::TSC3414Blue()
{
	uint8_t blueLow = 0;
	uint8_t blueHigh = 0;
	int ret = 0;
	if (connected_)
	{
		try
		{
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x94, &blueLow); //read Clear register
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x95, &blueHigh); //read Clear register
			logger().debug() << "blueLow: " << (int) blueLow << " \tblueHigh: " << (int) blueHigh << utils::end;
			blueHigh = (blueHigh * 256) + blueLow;
		} catch (utils::Exception * e)
		{
			logger().error() << "Exception TSC3414Blue: " << e->what() << utils::end;
		}
	}
	return blueHigh;
}

/*** Gets the green sensor value and returns an unsigned int ***/
uint8_t pmx::GroveColorSensor::TSC3414Green()
{
	uint8_t greenLow = 0;
	uint8_t greenHigh = 0;
	int ret = 0;
	if (connected_)
	{
		try
		{
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x90, &greenLow); //read Clear register
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x91, &greenHigh); //read Clear register
			logger().debug() << "greenLow: " << (int) greenLow << " \tgreenHigh: " << (int) greenHigh << utils::end;
			greenHigh = (greenHigh * 256) + greenLow;
		} catch (utils::Exception * e)
		{
			logger().error() << "Exception TSC3414Green: " << e->what() << utils::end;
		}
	}
	return greenHigh;
}

/*** Gets the red sensor value and returns an unsigned int ***/
uint8_t pmx::GroveColorSensor::TSC3414Red()
{
	uint8_t redLow = 0;
	uint8_t redHigh = 0;
	int ret = 0;
	if (connected_)
	{
		try
		{
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x92, &redLow); //read Clear register
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x93, &redHigh); //read Clear register
			logger().debug() << "redLow: " << (int) redLow << " \tredHigh: " << (int) redHigh << utils::end;
			redHigh = (redHigh * 256) + redLow;
		} catch (utils::Exception * e)
		{
			logger().error() << "Exception TSC3414Red: " << e->what() << utils::end;
		}
	}
	return redHigh;
}

/*** Gets the clear sensor value and returns an unsigned int ***/
uint8_t pmx::GroveColorSensor::TSC3414Clear()
{
	uint8_t clearLow = 0;
	uint8_t clearHigh = 0;
	int ret = 0;
	if (connected_)
	{
		try
		{
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x96, &clearLow); //read Clear register
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x97, &clearHigh); //read Clear register
			logger().debug() << "clearH: " << (int) clearLow << "  \tclearL: " << (int) clearHigh << utils::end;
			clearHigh = (clearHigh * 256) + clearLow;
		} catch (utils::Exception * e)
		{
			logger().error() << "Exception TSC3414Clear : " << e->what() << utils::end;
		}
	}
	return clearHigh;
}

/*
 * ======================================================
 * Sensor read functions - retrieves the RGBW raw sensor values
 * ======================================================
 */
void pmx::GroveColorSensor::TSC3414All(uint8_t allcolors[])
{
	uint8_t white = TSC3414Clear();
	uint8_t green = TSC3414Green();
	uint8_t red = TSC3414Red();
	uint8_t blue = TSC3414Blue();

	allcolors[0] = white;
	allcolors[1] = red;
	allcolors[2] = green;
	allcolors[3] = blue;
	//returns all colors;
}

/*
 * ======================================================
 * TCS3414 Initialization
 * Turns on the sensor and sets integration time
 * ======================================================
 */
void pmx::GroveColorSensor::TCS3414Initialize(int delay1, int delay2)
{
	CMD(0); //log
	int ret = 0;
	if (connected_)
	{
		try
		{
			//slave address: 0011 1001
			//0x80 1000 0000 //write to Control register
			//0x01 0000 0001 //Turn the device on (does not enable ADC yet)
			ret = utils::HostI2cBus::instance().writeRegisterbyte(0x80, 0x01);

			usleep(delay1 * 1000); //14

			// Request confirmation //0011 1001
			uint8_t receivedVal; //0001 (ADC valid) 0001 (Power on)
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x39, &receivedVal);

			// Request ID //0011 1001
			uint8_t ID;
			//0x84 1000 0100 //get information from ID register (04h)
			ret = utils::HostI2cBus::instance().readRegisterbyte(0x84, &ID);
			//0001 0000 (first byte == 0001 (TCS: 3413,3414,3415,3416) or 0000 (TCS: 3404).

			if (ID == 1 || ID == 17)
			{ //0000 0001 || 0001 0001
				logger().debug() << "TCS3414 is now ON" << utils::end;
			}
			else
			{
				logger().debug() << "TCS3414 is now OFF" << utils::end;
			}

			//Write to Timing (integration) register
			if (integrationtime_ == 12)
			{
				//0000 0000 //set free running INTEG_MODE and integration time to 12ms
				ret = utils::HostI2cBus::instance().writeRegisterbyte(0x81, 0x00);
			}
			else if (integrationtime_ == 100)
			{
				//0000 0001 //set free running INTEG_MODE and integration time to 100ms
				ret = utils::HostI2cBus::instance().writeRegisterbyte(0x81, 0x01);
			}
			else if (integrationtime_ == 400)
			{
				//0000 0010 //set free running INTEG_MODE and integration time to 400ms
				ret = utils::HostI2cBus::instance().writeRegisterbyte(0x81, 0x02);
			}
			else
			{
				logger().debug() << "improper integration time has been set!" << utils::end;
			}

			//0000 0011 //Enable ADC_EN (needed to allow integration every 100ms)
			ret = utils::HostI2cBus::instance().writeRegisterbyte(0x80, 0x03);
		} catch (utils::Exception * e)
		{
			logger().error() << "Exception TCS3414Initialize : " << e->what() << utils::end;
		}
		usleep(delay2 * 1000); //14 //wait for a moment to allow ADC to initialize*/
	}
}

/*** Keeps a running average of 4 values per color. ***/
void pmx::GroveColorSensor::calculateMedium(float med[], uint8_t value[], float divider)
{
	for (int i = 0; i < 4; i++)
	{
		med[i] = ((med[i] * (divider - 1.0)) + value[i]) / divider;
	}
}

/*** calculates percentages for R,G,B channels, if enabled.  ***/
void pmx::GroveColorSensor::makePercentage(uint8_t allcolors[], float allmedium[])
{ //makes every color a percentage, 100% is the average of the previous 4 values before this is entered.
	for (int i = 0; i < 4; i++)
	{
		allcolors[i] = (int) (allcolors[i] / allmedium[i] * 100);
	}
}

//compensateEnabled = false; //enable/disable color compensation of the sensor sensitivity per color
void pmx::GroveColorSensor::colorCompensator(uint8_t allcolors[])
{
	allcolors[2] = (int) (allcolors[2] * 1.3125); //green
	allcolors[3] = (int) (allcolors[2] * 1.5973); //blue
	//look at the graph in the datasheet to find the following percentages
	//blue  52%    (r/b == 1.5973)
	//green 63.28% (r/g == 1.3125)
	//red   83.06%
}

/*** takes the raw values from the sensors and converts them to
 Correlated Color Temperature.  Returns a float with CCT ***/
float pmx::GroveColorSensor::CCTCalc(uint8_t allcolors[])
{
	float TCS3414tristimulus[3]; // [tri X, tri Y, tri Z]
	float TCS3414chromaticityCoordinates[2]; //chromaticity coordinates // [x, y]

	//calculate tristimulus values (chromaticity coordinates)
	//The tristimulus Y value represents the illuminance of our source
	TCS3414tristimulus[0] = (-0.14282 * allcolors[1]) + (1.54924 * allcolors[2]) + (-0.95641 * allcolors[3]); //X
	TCS3414tristimulus[1] = (-0.32466 * allcolors[1]) + (1.57837 * allcolors[2]) + (-0.73191 * allcolors[3]); //Y // = Illuminance
	TCS3414tristimulus[2] = (-0.68202 * allcolors[1]) + (0.77073 * allcolors[2]) + (0.56332 * allcolors[3]); //Z

	float XYZ = TCS3414tristimulus[0] + TCS3414tristimulus[1] + TCS3414tristimulus[2];

	//calculate the chromaticiy coordinates
	TCS3414chromaticityCoordinates[0] = TCS3414tristimulus[0] / XYZ; //x
	TCS3414chromaticityCoordinates[1] = TCS3414tristimulus[1] / XYZ; //y

	float n = (TCS3414chromaticityCoordinates[0] - 0.3320) / (0.1858 - TCS3414chromaticityCoordinates[1]);

	float CCT = ((449 * pow(n, 3)) + (3525 * pow(n, 2)) + (6823.3 * n) + 5520.33);

	logger().debug() << "Illuminance: " << TCS3414tristimulus[1] << " \tx: " << TCS3414chromaticityCoordinates[0]
			<< " \ty: " << TCS3414chromaticityCoordinates[1] << " \tCCT:  " << CCT << " K" << utils::end;

	return CCT;
}

// [Clear,Red,Green,Blue]
uint8_t * pmx::GroveColorSensor::TCS3414GetValues()
{
	float ColorTemperature = 0;

	//gets the raw values from the sensors and writes it to TCS3414values[]
	TSC3414All(TCS3414values_);

	//compensate based on the filter characteristics of the TCS3414
	if (compensateEnabled_)
		colorCompensator(TCS3414values_);

	//keeps a running average from the last 4 values per color.
	calculateMedium(TCS3414mediate_, TCS3414values_, 4.0);

	//calculates the color temperature, using the algorithm in the TCS3414 datasheet
	ColorTemperature = CCTCalc(TCS3414values_);

	//displays percentage values, if enabled.
	if (percentageEnabled_)
	{
		makePercentage(TCS3414values_, TCS3414medium_);
	}

	logger().info() << "Clear: " << (int) TCS3414values_[0] << " \tRed: " << (int) TCS3414values_[1] << " \tGreen: "
			<< (int) TCS3414values_[2] << " \tBlue:  " << (int) TCS3414values_[3] << utils::end;

	//pause
	usleep(loopdelay_ * 1000); //delays by the integration time between measurements

	return TCS3414values_;
}

void pmx::GroveColorSensor::CMD(int delayTime)
{

	if (percentageEnabled_)
	{
		logger().debug() << "TSC3414 Percentage mode ON" << utils::end;
	}
	else
	{
		logger().debug() << "TSC3414 Percentage mode OFF" << utils::end;
	}

	if (compensateEnabled_)
	{
		logger().debug() << "TSC3414 Color compensation mode ON" << utils::end;
	}
	else
	{
		logger().debug() << "TSC3414 Color compensation mode OFF" << utils::end;
	}

	usleep(delayTime * 1000);
}

