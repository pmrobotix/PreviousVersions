/*!
 * \file
 * \brief Implémentation de la classe LedBarTest.
 */

#include "../Common/Action/LedBar.hpp"
#include "LedBarTest.hpp"

#include <unistd.h>

#include "../Bot-SmallPMX/SActionsExtended.hpp"
#include "../Bot-SmallPMX/SRobotExtended.hpp"
#include "../Common/Action.Driver/ALedDriver.hpp"
#include "../Common/LogAppender/Logger.hpp"


using namespace std;

void LedBarTest::run(int, char *[])
{
	logger().info() << "Executing - " << this->desc() << logs::end;

	SRobotExtended robot;

	robot.actions.ledbar().resetAll();

/*
	robot.actions.ledbar().setOff(0);
	robot.actions.ledbar().setOff(1);
	usleep(500000);
	robot.actions.ledbar().setOn(0);
	robot.actions.ledbar().setOn(1);
	usleep(500000);
	robot.actions.ledbar().setOff(0);
	robot.actions.ledbar().setOff(1);
	usleep(500000);
	robot.actions.ledbar().set(0, LED_RED);
	usleep(1000000);
	robot.actions.ledbar().set(0, LED_ORANGE);
	usleep(1000000);
	robot.actions.ledbar().set(0, LED_GREEN);
	usleep(1000000);
	robot.actions.ledbar().setOff(0);
	usleep(1000000);

	robot.actions.ledbar().blink(5, 200000, LED_ORANGE);

	robot.actions.ledbar().k2mil(4, 100000, LED_RED);
*/
/*
	robot.actions.start();

	//robot.actions.ledbar().startSet(0,1);
	//robot.actions.ledbar().startAlternate(10, 500000, 0x03, 0x01, LED_GREEN, true);

	robot.actions.ledbar().startK2mil(2, 100000, LED_ORANGE, true);
	robot.actions.ledbar().startBlinkPin(10, 500000, 4, LED_ORANGE, true);
	robot.actions.stop();*/

	robot.actions.ledbar().resetAll();

	logger().info() << "Happy End." << logs::end;
}

