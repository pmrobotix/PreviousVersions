/*!
 * \file
 * \brief Définition des macros du projet.
 */

#ifndef PMX_MACRO_HPP
#define PMX_MACRO_HPP

/*!
 * \brief Type de données : unsigned int.
 */
typedef unsigned int uint;

/*!
 * \brief Type de données : unsigned short.
 */
typedef unsigned short ushort;

/*!
 * \brief Type de données : unsigned long.
 */
typedef unsigned long ulong;

namespace pmx
{

    /*!
     * \brief Enumération du mode de programmation.
     */
    enum RobotMode
    {
        ROBOTMATCHES, ROBOTHOMOLOGATION
    };

}
#endif