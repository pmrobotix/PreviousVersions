/*!
 * \file
 * \brief Implémentation de la classe GpioBoard.
 */

#include "GpioBoard.hpp"

#include <unistd.h>

#include "HostI2cBus.hpp"


pmx::GpioBoard::GpioBoard(pmx::Robot & robot)
		: ARobotElement(robot), connected_(false)
{
	try
	{
		utils::HostI2cBus::instance().open(); //TODO close it by the robot destructor
		connected_ = true;

	} catch (utils::I2cException * e)
	{
		logger().error() << "Exception open: " << e->what() << utils::end;
	} catch (utils::I2cWarning * e)
	{
		logger().debug() << "Exception open: " << e->what() << utils::end;
	}

}

void pmx::GpioBoard::setup()
{
	write_i2c(CONFIG_P0, 0x00); //defines all pins on Port0 are outputs
	write_i2c(CONFIG_P1, 0x00); //defines all pins on Port1 are outputs
	write_i2c(OUT_P0, 0x00); //clears all relays
	write_i2c(OUT_P1, 0x00); //clears all relays
	usleep(PAUSE);
}

void pmx::GpioBoard::setValueP0(int port, int pin, int value)
{
	int out = 0;

	if (value == 1)
		out = port0Value_ | (0x01 << pin);
	else if (value == 0)
		out = port0Value_ & (0xFE << pin);

	write_i2c(port, out);
}

void pmx::GpioBoard::setOnP0(int pin) // 0 à 7
{
	setValueP0(OUT_P0, pin, 1);
}

void pmx::GpioBoard::setOffP0(int pin) // 0 à 7
{
	setValueP0(OUT_P0, pin, 0);
}

int pmx::GpioBoard::getValue()
{
	return 0;
}


void pmx::GpioBoard::write_i2c(uchar command, uchar value)
{
	try
	{
		utils::HostI2cBus::instance().writeRegValue(GPIOBOARD_PCA9555, command, value);

	} catch (utils::Exception * e)
	{
		logger().error() << "Exception GroveColorSensor::write_i2c: " << e->what() << utils::end;
	}
}

uchar pmx::GpioBoard::read_i2c(uchar command)
{
	uchar receivedVal = 0;
	try
	{
		utils::HostI2cBus::instance().readRegValue(GPIOBOARD_PCA9555, command, &receivedVal);

	} catch (utils::Exception * e)
	{
		logger().error() << "Exception GroveColorSensor::read_i2c: " << e->what() << utils::end;
	}
	return receivedVal;
}

/*
void pmx::GpioBoard::write_i2c(uchar command, uchar value)
{
	try
	{
		utils::HostI2cBus::instance().writeRegValue(GPIOBOARD_PCA9555, command, value);

	} catch (utils::Exception * e)
	{
		logger().error() << "Exception GpioBoard::write_i2c: " << e->what() << utils::end;
	}
}

uint8_t pmx::GpioBoard::read_i2c(uchar command)
{
	try
	{
		uint8_t receivedVal = 0;
		utils::HostI2cBus::instance().readRegValue(GPIOBOARD_PCA9555, command, &receivedVal);
		return receivedVal;

	} catch (utils::Exception * e)
	{
		logger().error() << "Exception GpioBoard::read_i2c: " << e->what() << utils::end;
	}
	return 0;
}
*/
