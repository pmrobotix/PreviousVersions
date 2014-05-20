/*!
 * \file
 * \brief Implémentation de la classe FindPIDTest..
 */

#include "FindPIDTest.hpp"

#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include "../../common/c/encoder.h"
#include "../../common/c/robot.h"
#include "../../common/cpp/Base.hpp"
#include "../../common/cpp/Logger.hpp"
#include "../../common/cpp/Robot.hpp"

void robottest::FindPIDTest::run(int argc, char *argv[])
{
	float Ap = 0.0;
	float Ai = 0.0;
	float Ad = 0.0;
	float Dp = 0.0;
	float Di = 0.0;
	float Dd = 0.0;
	int sec = 5;
	int dist = 0;
	float degrees = 0;

	if (argc < 11)
	{
		std::cout << "USAGE: PMX_TEST " << argv[1]
				<< " [alpha degrees] [P] [I] [D]  [delta mm] [P] [I] [D] [option:sec:5s]" << std::endl;
	}

	if (argc > 2)
	{
		degrees = atof(argv[2]);
		std::cout << "degrees: " << atof(argv[2]) << std::endl;
	}
	else
	{
		std::cout << "degrees: " << std::flush;
		std::cin >> degrees;
	}

	if (argc > 3)
	{
		Ap = atof(argv[3]);
		std::cout << "Ap: " << atof(argv[3]) << std::endl;
	}
	else
	{
		std::cout << "Ap: " << std::flush;
		std::cin >> Ap;
	}

	if (argc > 4)
	{
		Ai = atof(argv[4]);
		std::cout << "Ai: " << atof(argv[4]) << std::endl;
	}
	else
	{
		std::cout << "Ai: " << std::flush;
		std::cin >> Ai;
	}

	if (argc > 5)
	{
		Ad = atof(argv[5]);
		std::cout << "Ad: " << atof(argv[5]) << std::endl;
	}
	else
	{
		std::cout << "Ad: " << std::flush;
		std::cin >> Ad;
	}

	if (argc > 6)
	{
		dist = atoi(argv[6]);
		std::cout << "dist: " << atoi(argv[6]) << std::endl;
	}
	else
	{
		std::cout << "dist: " << std::flush;
		std::cin >> dist;
	}

	if (argc > 7)
	{
		Dp = atof(argv[7]);
		std::cout << "Dp: " << atof(argv[7]) << std::endl;
	}
	else
	{
		std::cout << "Dp: " << std::flush;
		std::cin >> Dp;
	}

	if (argc > 8)
	{
		Di = atof(argv[8]);
		std::cout << "Di: " << atof(argv[8]) << std::endl;
	}
	else
	{
		std::cout << "Di: " << std::flush;
		std::cin >> Di;
	}

	if (argc > 9)
	{
		Dd = atof(argv[9]);
		std::cout << "Dd: " << atof(argv[9]) << std::endl;
	}
	else
	{
		std::cout << "Dd: " << std::flush;
		std::cin >> Dd;
	}

	if (argc > 10)
	{
		sec = atoi(argv[10]);
		std::cout << "sec: " << atoi(argv[10]) << std::endl;
	}

	logger().info() << "-------------------------------------------------" << utils::end;
	logger().info() << "FindPIDTest - " << utils::end;

	pmx::Robot &robot = pmx::Robot::instance();
	//sleep(1);
	//robot.start();

	int lRes = 1121;
	int rRes = 1192;
	float distRes = 0.300f;

	if (useExternalEncoders) //TODO a mettre dans Base.cpp
	{
		lRes = 19885;
		rRes = 20360;
		distRes = 0.250f;
	}
	else
	{
		lRes = 1136;
		rRes = 1136;
		distRes = 0.300f;

	}
	/*
	 long left = robot_getLeftInternalCounter();
	 long right = robot_getRightInternalCounter();
	 logger().info() << "L=" << left << "\tR=" << right << utils::end;

	 robot_setMotorLeftSpeed(100);
	 robot_setMotorRightSpeed(100);
	 sleep(3);
	 left = robot_getLeftInternalCounter();
	 right = robot_getRightInternalCounter();
	 logger().info() << "L=" << left << "\tR=" << right << utils::end;
	 */

	robot.base().begin(lRes, rRes, distRes, 1);
	robot.base().setupPID_AD(Ap, Ai, Ad, Dp, Di, Dd);

	robot.base().printPosition();
	robot.base().findPidAD(degrees, dist, sec);

	robot.stop();
	robot.base().printPosition();

	logger().info() << "End of RobotTest." << utils::end;
}

