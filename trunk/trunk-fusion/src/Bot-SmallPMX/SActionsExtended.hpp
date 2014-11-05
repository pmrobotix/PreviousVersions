#ifndef PMX_S_ACTIONSEXTENDED_HPP_
#define PMX_S_ACTIONSEXTENDED_HPP_

#include "../Common/Action/LedBar.hpp"
#include "../Common/Action/Actions.hpp"

class SActionsExtended: public Actions
{
public:

	/*!
	 * \brief LED Bar.
	 */
	LedBar ledbar_;


	SActionsExtended() :
			ledbar_(2)
	{
	}

};

#endif
