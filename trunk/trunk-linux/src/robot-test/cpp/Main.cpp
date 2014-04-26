/*!
 * \file
 * \brief Ce fichier contient le programme principal pour le lancement des tests
 * sur le robot.
 */

#include <iostream>
#include "../../common/cpp/ConsoleUnitTestManager.hpp"
#include "../../common/cpp/LoggerFactory.hpp"

#include "LedIndicatorTest.hpp"
#include "LedBarTest.hpp"
#include "GroveColorTest.hpp"
#include "ExtEncoderTest.hpp"
#include "ServoMotorStdTest.hpp"
#include "IrSensorsGroupTest.hpp"
#include "ServoMotorDxlTest.hpp"
#include "ArduinoTest.hpp"
#include "Md25Test.hpp"

/*!
 * \brief Méthode principale pour le lancement des tests sur le robot.
 *
 * \param argc
 *        Nombre d'arguments.
 * \param argv
 *        Arguments du programme (argv[0] est le nom de l'application).
 * \return Code de retour du programme.
 */
int main(int argc, char** argv)
{
	//std::cout << "main robot-test Start" << std::endl;
	//const utils::Logger& logger = utils::LoggerFactory::logger("main");
	//logger.info("Lancement des tests unitaires");

	pmx::ConsoleUnitTestManager manager;

	manager.add(new robottest::LedIndicatorTest());
	manager.add(new robottest::LedBarTest());
	manager.add(new robottest::GroveColorTest());
	manager.add(new robottest::ExtEncoderTest());
	manager.add(new robottest::ServoMotorStdTest());
	manager.add(new robottest::IrSensorsGroupTest());
	manager.add(new robottest::ServoMotorDxlTest());
	manager.add(new robottest::ArduinoTest());
	manager.add(new robottest::Md25Test());

	if (argc == 1)
	{
		manager.run();
	}
	else
	{
		manager.run(atoi(argv[1]), argc, argv);
	}

}
