/*!
 * \file
 * \brief Définition de la classe ApfInModeTest.
 */

#ifndef TEST_APFINMODETEST_HPP
#define TEST_APFINMODETEST_HPP

#include <string>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../common/cpp/ConsoleUnitTest.hpp"
#include "../../common/cpp/LoggerFactory.hpp"
#include "../../common/cpp/Chronometer.hpp"

namespace test
{
    /*!
     * \brief Effectue un test de clignotement de LED sur tous les pads de sorties de la carte DevLightV2.
     */
    class UTGpioWithModeTest : public pmx::ConsoleUnitTest
    {
    private:

        static std::string INPUT_PORT_PATH_DEV;
        static std::string INPUT_PORT_PATH_PROC;

         /*!
         * \brief Retourne le \ref Logger associé à la classe \ref ApfInModeTest.
         */
        static inline const utils::Logger & logger()
        {
            static const utils::Logger & instance = utils::LoggerFactory::logger("test::UTGpioWithModeTest");
            return instance;
        }

    public:

        /*!
         * \brief Constructeur de la classe.
         */
        UTGpioWithModeTest() : pmx::ConsoleUnitTest("APFIN  : ApfInModeTest : Test de differents modes de -Read- et -Blocking Read-.")
        {
        }

        /*!
         * \brief Destructeur de la classe.
         */
        virtual ~UTGpioWithModeTest()
        {
        }

        /*!
         * \brief Execute le test.
         */
        virtual void run(int argc, char *argv[]);

        void in_V1_blockingReadFstream(std::string inGpioPort, std::string inPin);
        void in_V2_blockingread(std::string inGpioPort, std::string inPin);
        void in_V3_ioctl_for_pin(std::string inGpioPort, std::string inPin);
        void in_V4_ioctl_for_port(std::string inGpioPort, std::string inPin);

    };
};

#endif
