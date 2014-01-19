/*!
 * \file
 * \brief Implémentation de la classe RobotFindPidPositionTest.
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include "RobotFindPidPositionTest.hpp"
#include "Robot.hpp"
#include "MotorWay.hpp"
#include "Md25.hpp"

using namespace std;
using namespace pmx;

void test::RobotFindPidPositionTest::run(int argc, char *argv[])
{
    std::cout << "RobotFindPidPositionTest: Start" << std::endl;
    //logger().info() << "test::RobotFindPidPositionTest: Start" << utils::end;
    int startTime, x, y;
    double leftP, leftI, leftD;
    double rightP, rightI, rightD;

    if (argc < 10)
    {
        std::cout << "USAGE: PMX_TEST " << argv[1] << " [Tps(sec)] [DistX(mm)] [EcartY(mm)] [PID_G] [PID_D]" << std::endl;
    }   

    if (argc > 2)
    {
        startTime = atoi(argv[2]);
        std::cout << "Temps d'arret en sec: " << atoi(argv[2]) << std::endl;
    }else
    {
        //demande du temps de marche
        std::cout << "Temps d'arret en sec: " << std::flush;
        std::cin >> startTime;
    }

    if (argc > 3)
    {
        x = atoi(argv[3]);
        std::cout << "x (en mm): " << atoi(argv[3]) << std::endl;
    }else
    {
        std::cout << "x (en mm): " << std::flush;
        std::cin >> x;
    }
    if (argc > 4)
    {
        y = atoi(argv[4]);
        std::cout << "Ecart y (en mm): (0)" << atoi(argv[4]) << std::endl;
    }else
    {
        std::cout << "Ecart y (en mm): (0)" << std::flush;
        std::cin >> y;
    }

    if (argc > 7)
    {
        leftP = atof(argv[5]);
        leftI = atof(argv[6]);
        leftD = atof(argv[7]);
        std::cout << "Gauche P: " << atof(argv[5]) << std::endl;
        std::cout << "       I: " << atof(argv[6]) << std::endl;
        std::cout << "       D: " << atof(argv[7]) << std::endl;
    }else
    {
        std::cout << "Gauche P: " << std::flush;
        std::cin >> leftP;
        std::cout << "       I: " << std::flush;
        std::cin >> leftI;
        std::cout << "       D: " << std::flush;
        std::cin >> leftD;
    }

    if (argc > 10)
    {
        rightP = atof(argv[8]);
        rightI = atof(argv[9]);
        rightD = atof(argv[10]);
        std::cout << "Droit  P: " << atof(argv[8]) << std::endl;
        std::cout << "       I: " << atof(argv[9]) << std::endl;
        std::cout << "       D: " << atof(argv[10]) << std::endl;
    }else
    {
        std::cout << "Droit  P: " << std::flush;
        std::cin >> rightP;
        std::cout << "       I: " << std::flush;
        std::cin >> rightI;
        std::cout << "       D: " << std::flush;
        std::cin >> rightD;
    }
    
    logger().debug() << "go" << utils::end;
    pmx::Robot robot; 
   
    //config PID
    robot.base().rampPositionEnabled(false);
    robot.base().vecteurCorrectionEnabled(false);
    robot.base().pidSpeedEnabled(false);
    robot.base().pidPositionEnabled(false);

    logger().debug() << "robot.start()" << utils::end;
    robot.start(); //lancement des threads
    //utils::Md25::instance().setCommand(MD25_ENABLE_SPEED_REGULATION); //activation du pid MD25
    //utils::Md25::instance().setAccelerationRate(10);
    robot.base().pidSpeedStart(); //start le timer de gestion PID
    robot.chronometerRobot().start(); //Chronometre général (match) du robot
/*
    if (x < 0)
    {
        robot.base().pidSpeedBackInitialize(pmx::LEFT, 40, 16.5, 0.1);
        robot.base().pidSpeedBackInitialize(pmx::RIGHT, 42, 16, 0.1);
    }else
    {
        robot.base().pidSpeedInitialize(pmx::LEFT, 42, 16.5, 0.1);
        robot.base().pidSpeedInitialize(pmx::RIGHT, 34, 13.4, 0.1);
    }
*/
    //logger().debug() << "configure(robot_pmxtest.conf)" << utils::end;
    //robot.configure("robot_pmxtest.conf");
    
    robot.base().pidPositionInitialize(pmx::MLEFT, leftP, leftI, leftD);
    robot.base().pidPositionInitialize(pmx::MRIGHT,rightP, rightI, rightD);

    robot.base().pidPositionTest(true);
    

    robot.base().moveD(x / pmx::PositionManager::DISTANCE_PER_MOTOR_PULSE, y / pmx::PositionManager::DISTANCE_PER_MOTOR_PULSE);

    logger().debug() << "Arrived or Sleep(Time) : " << startTime << " Sec" << utils::end;

    while(robot.base().arrived() == false && robot.chronometerRobot().getElapsedTimeInSec() < startTime)
    {
        usleep(10000);
    }
    robot.base().stop();
    logger().info() << "Stop" << utils::end;
    
    logger().info() << "Arrived    x: " << robot.position().x() << " mm y: " << robot.position().y()
                << " mm angle: "<< robot.position().angle() * 180.0 / M_PI
                << " degré time(s): " << robot.chronometerRobot().getElapsedTimeInSec() << utils::end;
    robot.chronometerRobot().stop();
    robot.stop();

    logger().info() << "End-Of-PMX : "
            << robot.chronometerRobot().getElapsedTimeInMicroSec() << " us "
            << utils::end;
}
