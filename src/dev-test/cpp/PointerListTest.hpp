/*!
 * \file
 * \brief Définition de la classe PointerListTest.
 */

#ifndef TEST_POINTERLISTTEST_HPP
#define TEST_POINTERLISTTEST_HPP

#include "UnitTest.hpp"

namespace test
{

    /*!
     * \brief Teste la classe \ref utils::PointerList
     */
    class PointerListTest : public utils::UnitTest
    {
    public:

        /*!
         * \brief Constructeur de la classe.
         */
        PointerListTest() : utils::UnitTest("PointerListTest")
        {
        }

        /*!
         * \brief Destructeur de la classe.
         */
        virtual ~PointerListTest()
        {
        }

        virtual void suite();

        void testConstructorDefault();

        void testConstructorInitialized();
        
        void testConstructorCopy();
        
        void testConstructorRange();
        
        void testDestructor();
    };
}

#endif
