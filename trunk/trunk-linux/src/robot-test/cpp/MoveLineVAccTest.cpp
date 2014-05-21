/*!
 * \file
 * \brief Implémentation de la classe MoveLineVAccTest..
 */

#include "MoveLineVAccTest.hpp"

#include "../../common/c/encoder.h"
#include "../../common/cpp/Base.hpp"
#include "../../common/cpp/Logger.hpp"
#include "../../common/cpp/Robot.hpp"

void robottest::MoveLineVAccTest::run(int argc, char *argv[])
{
	int dist = 0;
	float vmax = 0;
	float acc = 0;
	float dec = 0;

	if (argc < 6)
	{
		std::cout << "USAGE: PMX_TEST " << argv[1] << " [dist] [Vmax] [Acc] [Dec]" << std::endl;
	}

	if (argc > 2)
	{
		dist = atoi(argv[2]);
		std::cout << "dist: " << atoi(argv[2]) << std::endl;
	}
	else
	{
		std::cout << "dist: " << std::flush;
		std::cin >> dist;
	}

	if (argc > 3)
	{
		vmax = atof(argv[3]);
		std::cout << "vmax: " << atof(argv[3]) << std::endl;
	}
	else
	{
		std::cout << "vmax: " << std::flush;
		std::cin >> vmax;
	}

	if (argc > 4)
	{
		acc = atof(argv[4]);
		std::cout << "acc: " << atof(argv[4]) << std::endl;
	}
	else
	{
		std::cout << "acc: " << std::flush;
		std::cin >> acc;
	}

	if (argc > 5)
	{
		dec = atof(argv[5]);
		std::cout << "dec: " << atof(argv[5]) << std::endl;
	}
	else
	{
		std::cout << "dec: " << std::flush;
		std::cin >> dec;
	}

	logger().info() << "-------------------------------------------------" << utils::end;
	logger().info() << "MoveTest - " << utils::end;

	pmx::Robot &robot = pmx::Robot::instance();
	//sleep(1);
	robot.start();

	int lRes = 1191; //1121
	int rRes = 1192; //1192
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

	robot.base().printPosition();
	robot.base().begin(lRes, rRes, distRes, 1);

	robot.base().MoveLineSpeedAcc(dist, vmax, acc, dec); // 1.4

	robot.stop();
	robot.base().printPosition();
	sleep(2);
	robot.base().printPosition();
	logger().info() << "End of RobotTest." << utils::end;
}
