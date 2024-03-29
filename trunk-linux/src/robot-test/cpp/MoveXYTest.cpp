/*!
 * \file
 * \brief Implémentation de la classe MoveXYTest..
 */

#include "MoveXYTest.hpp"

#include <stdlib.h>
#include <iostream>

#include "../../common/c/ccbase.h"
#include "../../common/cpp/Base.hpp"
#include "../../common/cpp/Chronometer.hpp"
#include "../../common/cpp/IrSensorsGroup.hpp"
#include "../../common/cpp/Logger.hpp"
#include "../../common/cpp/Robot.hpp"

void robottest::MoveXYTest::run(int argc, char *argv[])
{
	//nb de coord
	int lot = argc / 4;
	int targetX[lot], targetY[lot], back[lot];
	double angle[lot];
	int color = 0, detect = 0;

	std::cout << "USAGE: PMX_TEST " << argv[1]
			<< " [colorMatch:0-1] [OpponentDetect:0-1]  [[coordX(mm)] [coordY(mm)] [angle(deg -179...+180)] [FORWARD-BACK:0-1] ...] "
			<< std::endl;

	if (argc > 2)
	{
		color = atoi(argv[2]);
		std::cout << "colorMatch:0-1: " << atoi(argv[2]) << std::endl;
	}
	else
	{
		std::cout << "colorMatch:0-1: " << std::flush;
		std::cin >> color;
	}

	if (argc > 3)
	{
		detect = atoi(argv[3]);
		std::cout << "OpponentDetect: " << atoi(argv[3]) << std::endl;
	}
	else
	{
		std::cout << "OpponentDetect (0-1): " << std::flush;
		std::cin >> detect;
	}

	int nb = 0;
	for (int i = 4; i < argc; i = i + 4)
	{
		if (argc > i)
		{
			targetX[nb] = atoi(argv[i]);
			std::cout << "coord-x (en mm): " << atoi(argv[i]) << std::endl;
		}
		else
		{
			std::cout << "coord-x (en mm): " << std::flush;
			std::cin >> targetX[nb];
		}

		if (argc > i + 1)
		{
			targetY[nb] = atoi(argv[i + 1]);
			std::cout << "coord-y (en mm): " << atoi(argv[i + 1]) << std::endl;
		}
		else
		{
			std::cout << "coord-y (en mm): " << std::flush;
			std::cin >> targetY[nb];
		}

		if (argc > i + 2)
		{
			angle[nb] = atoi(argv[i + 2]);
			std::cout << "angle (en degré): " << atoi(argv[i + 2]) << std::endl;
		}
		else
		{
			std::cout << "angle (en degré): " << std::flush;
			std::cin >> angle[nb];
		}

		if (argc > i + 3)
		{
			back[nb] = atoi(argv[i + 3]);
			std::cout << "go [FORWARD:0 BACK:1]: " << atoi(argv[i + 3]) << std::endl;
		}
		else
		{
			std::cout << "go [FORWARD:0 BACK:1]: " << std::flush;
			std::cin >> back[nb];
		}
		nb++;
	}

	logger().info() << "-------------------------------------------------" << utils::end;
	logger().info() << "MoveTest - " << utils::end;

	pmx::Robot &robot = pmx::Robot::instance();

	robot.base().setMatchColor(color);
	int asserv=1;
	robot.start(0, asserv);


	cc_setPosition(0, 0, 0.0, cc_getMatchColor());
	//cc_setPosition(220.0, 1021.0, 155.0, cc_getMatchColor());

	robot.base().printPosition();

	logger().info() << "cc_motion_GetDefaultSpeed() " << cc_motion_GetDefaultSpeed() << utils::end;


	if (detect == true)
	{
		robot.irSensorsGroup().startTimer();
	}
	for (int j = 0; j < nb; j++)
	{

		logger().info() << "Go    x:" << targetX[j] << " y:" << targetY[j] << " way:" << back[j] << " time(s):"
				<< robot.chronometerRobot().getElapsedTimeInSec() << utils::end; //TODO remettre le chrono

		robot.base().movexyteta(back[j], targetX[j], targetY[j], angle[j]);
		robot.base().printPosition();

	}

	if (detect == true)
	{
		robot.irSensorsGroup().stopTimer();
	}
	/*
	 logger().info() << "start Robot assistedHandling" << utils::end;
	 robot.base().assistedHandling(); //TODO regler le PID LEFT/RIGHT
	 sleep(5);
	 logger().info() << "Stop Robot assistedHandling" << utils::end;
	 */
	robot.stop();
	//sleep(1);
	robot.base().printPosition();
	logger().info() << "End of RobotTest." << utils::end;
}

