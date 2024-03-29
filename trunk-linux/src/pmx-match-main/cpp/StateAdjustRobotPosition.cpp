/*!
 * \file
 * \brief Implémentation de la classe StateAdjustRobotPosition.
 */

#include "StateAdjustRobotPosition.hpp"

#include <stdint.h>

#include "../../common/c/ccbase.h"
#include "../../common/c/robot.h"
#include "../../common/cpp/Adafruit_RGBLCDShield.hpp"
#include "../../common/cpp/LedBar.hpp"
#include "../../common/cpp/Logger.hpp"
#include "../../common/cpp/Robot.hpp"
#include "../../common/cpp/SvgWriter.hpp"
#include "Data.hpp"

//#include "StateInitialize.hpp"
//#include "StateWaitForReboot.hpp"
//#include "StateWaitForStart.hpp"

pmx::IAutomateState*
pmx::StateAdjustRobotPosition::execute(Robot& robot, void *data)
{
	IAutomateState* result;

	this->logger().info() << "start 03" << utils::end;
	pmx::Data* sharedData = (pmx::Data*) data;

	if (!sharedData->skipSetup())
	{
		robot.lcdBoard().setBacklight(LCD_ON);
		robot.lcdBoard().clear();
		robot.lcdBoard().print("PMX...Adjust Position !");
	}

	logger().debug() << "Adjust position in the correct board..." << utils::end;

	logger().debug() << "Color=" << robot.myColor() << " cc_getMatchColor()=" << cc_getMatchColor()
			<< utils::end;

	if (robot.myRunningMode() == ROBOTMATCHES)
	{
		//cc_setPosition(214.0, 1010.0, 150.5, cc_getMatchColor()); //entre 150 et 151°
		cc_setPosition(198.0, 1000.0, 0.0, cc_getMatchColor());
	}
	if (robot.myRunningMode() == ROBOTTABLETEST)
	{
		cc_setPosition(198.0, 1000.0, 0.0, cc_getMatchColor());
	}
	if (robot.myRunningMode() == ROBOTHOMOLOGATION)
	{
		cc_setPosition(198.0, 1000.0, 0.0, cc_getMatchColor());
	}

	logger().info() << "cc_getX()=" << cc_getX() << " cc_getY()=" << cc_getY()
			<< " cc_getThetaInDegree()=" << cc_getThetaInDegree() << utils::end;

	//Ecriture dans le SVG
	utils::SvgWriter::writePosition(cc_getX(), cc_getY(), cc_getTheta(), utils::SVG_POS_ROBOT);

	//skip setup
	if (!sharedData->skipSetup())
	{

		logger().info() << "=> Clic SELECT..." << utils::end;
		robot.lcdBoard().setCursor(0, 1);
		robot.lcdBoard().print("Clic SELECT...");
		robot.ledBar().startK2Mil(50000, 50000, false);
		//wait
		uint8_t buttons = 0;
		while (!(buttons & BUTTON_SELECT))
		{
			buttons = robot.lcdBoard().readButtons();
			if (buttons)
			{
				if (buttons & BUTTON_SELECT)
				{
					robot.lcdBoard().clear();
					robot.lcdBoard().setCursor(0, 0);
					robot.lcdBoard().print("NEXT ");
					//robot.lcdBoard().setBacklight(LCD_OFF);
				}

			}
		}

		robot.ledBar().startReset();
		robot.ledBar().stopAndWait(true);

		logger().debug() << "Position adjusted." << utils::end;
	}
	result = this->getState("next");

	return result;
}
