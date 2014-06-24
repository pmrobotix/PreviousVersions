/*!
 * \file
 * \brief Définition de la classe MoveXYTest.
 */

#ifndef ROBOTTEST_MOVEXYTEST_HPP
#define	ROBOTTEST_MOVEXYTEST_HPP

#include "../../common/cpp/ConsoleUnitTest.hpp"
#include "../../common/cpp/LoggerFactory.hpp"

namespace robottest
{
/*!
 * \brief Effectue un test d'avance.
 */
class MoveXYTest: public pmx::ConsoleUnitTest
{
private:

	/*!
	 * \brief Retourne le \ref Logger associé à la classe \ref MoveXYTest.
	 */
	static inline const utils::Logger & logger()
	{
		static const utils::Logger & instance = utils::LoggerFactory::logger("robottest::MoveXYTest");
		return instance;
	}

public:

	/*!
	 * \brief Constructeur de la classe.
	 */
	MoveXYTest()
			: pmx::ConsoleUnitTest("MoveXYTest : ")
	{
	}

	/*!
	 * \brief Destructeur de la classe.
	 */
	virtual ~MoveXYTest()
	{
	}

	/*!
	 * \brief Execute le test.
	 */
	virtual void run(int argc, char *argv[]);
};
}


#endif
