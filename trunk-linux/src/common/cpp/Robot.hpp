/*!
 * \file
 * \brief Définition de la classe Robot.
 */

#ifndef PMX_ROBOT_HPP
#define PMX_ROBOT_HPP

#include "IInitialized.hpp"
#include "Base.hpp"
#include "Chronometer.hpp"
#include "RobotColor.hpp"
#include "GroveColorSensor.hpp"
#include "LedIndicator.hpp"
#include "ActionManager.hpp"
#include "LedBar.hpp"
#include "Md25.hpp"

namespace pmx
{
/*!
 * \brief Cette classe représente le robot.
 *
 * Cette classe maitresse de l'application représente le robot et permet
 * d'accéder à l'ensemble de ses éléments.
 */
class Robot: public IInitialized
{
private:

	/*!
	 * \brief Retourne le \ref Logger associé à la classe \ref Robot.
	 */
	static inline const utils::Logger & logger()
	{
		static const utils::Logger & instance = utils::LoggerFactory::logger("pmx::Robot");
		return instance;
	}
	/*!
	 * \brief Assure la gestion des actions du robot.
	 */
	pmx::ActionManager actionManager_;

	/*!
	 * \brief chronometerRobot.
	 */
	utils::Chronometer chronometerRobot_;

	/*!
	 * \brief Base roulante du robot.
	 */
	pmx::Base base_;

	/*!
	 * \brief Couleur du robot.
	 */
	pmx::RobotColor myColor_;

	/*!
	 * \brief GroveColorSensor.
	 */
	pmx::GroveColorSensor groveColorSensor_;

	/*!
	 * \brief Barre de LED.
	 */
	pmx::LedBar ledBar_;

	/*!
	 * \brief MotorisationMd25.
	 */
	pmx::Md25 md25_;

public:

	/*!
	 * \brief Constructeur de la classe.
	 */
	Robot();

	/*!
	 * \brief Destructeur de la classe.
	 */
	virtual inline ~Robot()
	{
	}

	/*!
	 * \brief Cette méthode retourne l'objet de manipulation de la base
	 * roulante du robot.
	 * \return La base roulante du robot.
	 */
	inline pmx::Base & base()
	{
		return base_;
	}

	/*!
	 * \brief Retourne la couleur du robot.
	 */
	inline pmx::RobotColor myColor() const
	{
		return myColor_;
	}

	/*!
	 * \brief Enregistre la couleur du robot.
	 */
	inline void myColorIs(pmx::RobotColor color)
	{
		this->myColor_ = color;
	}

	/*!
	 * \brief Cette methode retourne l'objet de manipulation du chronometer.
	 * \return Le chronometer.
	 */
	inline utils::Chronometer & chronometerRobot()
	{
		return chronometerRobot_;
	}

	/*!
	 * \brief Cette methode retourne l'objet de manipulation du GroveColorSensor.
	 * \return Le GroveColorSensor.
	 */
	inline pmx::GroveColorSensor & groveColorSensor()
	{
		return groveColorSensor_;
	}

	/*!
	 * \brief Cette methode retourne l'objet de manipulation du ledBar.
	 * \return Le ledBar.
	 */
	inline pmx::LedBar & ledBar()
	{
		return ledBar_;
	}

	/*!
		 * \brief Cette methode retourne l'objet de manipulation du ledBar.
		 * \return Le ledBar.
		 */
		inline pmx::Md25 & md25()
		{
			return md25_;
		}

	/*!
	 * \brief Ajout d'une action.
	 * \param action
	 *        L'action à ajouter.
	 */
	inline void addAction(IAction * action)
	{
		logger().debug() << "Robot addAction" << utils::end;
		actionManager_.addAction(action);
	}

	/*!
	 * \brief Active le robot.
	 *
	 * Cette méthode lance les différents threads associés et initialise
	 * le système.
	 */
	void start();

	/*!
	 * \brief Arrete le robot et libère les ressources associés.
	 */
	void stop();

	/*!
	 * \brief Arrete le thread sensorManager et actionManager.
	 */
	void stopManagers();

	/*!
	 * \brief Arrete les composant du robot.
	 */
	void stopDevices();

	/*!
	 * \brief Initialise les paramètres du robot via un fichier de configuration.
	 */
	virtual void initialize(const std::string & prefix, utils::Configuration & configuration);
	/*!
	 * \brief Lance l'initialisation par fichier de configuration
	 */
	void configure(const std::string & configurationFile);

	void goTo(double x, double y, BaseWay way, bool detection);

	void goToTeta(double x, double y, double teta, BaseWay way, bool detection);
};
}

#endif