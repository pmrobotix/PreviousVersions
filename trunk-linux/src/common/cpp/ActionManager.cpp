/*!
 * \file
 * \brief Implémentation de la classe ActionManager.
 */

#include "ActionManager.hpp"

#include <stddef.h>
#include <unistd.h>
#include <list>

#include "Logger.hpp"

pmx::ActionManager::ActionManager()
		: stop_(false)
{
}

void pmx::ActionManager::execute()
{
	logger().info("ActionManager is started");

	int size = 0;
	while (!stop_)
	{
		lock();
		size = actions_.size();
		unlock();
		if (size > 0)
		{
			lock();
			IAction * action = actions_.front();
			if (action == NULL)
			{
				logger().error("ActionManager NULL action");
			}
			actions_.pop_front();
			unlock();
			//bool persist = action->execute(out);
			bool persist = action->execute();
			lock();
			if (persist)
			{
				actions_.push_back(action);
			}
			unlock();
		}
		else
		{
			// Comme il n'y a pas d'action, le processus attend avant de
			// relancer la boucle d'execution
			usleep(1000);
		}
	}

	logger().info("ActionManager is stopped");
}

void pmx::ActionManager::debug()
{
	lock();
	logger().info() << "Defined actions" << utils::end;
	utils::PointerList<pmx::IAction *>::iterator i = actions_.begin();
	while (i != actions_.end())
	{
		pmx::IAction * action = *i;
		logger().info() << " - " << action->info() << utils::end;
		i++;
	}
	unlock();
	logger().info() << "End of defined actions" << utils::end;
}
