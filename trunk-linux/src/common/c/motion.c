/*******************************************************************************
p * ClubElek Robot motion control software for Eurobot 2007
 * Copyright (C) 2006-2007 ClubElek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************************/
//! \file motion.c
//! \author Julien Rouviere <gmail address : julien.rouviere@...>
//! \author ClubElek <http://clubelek.insa-lyon.fr>
// svn :
// $LastChangedBy$
// $LastChangedDate$
/******************************************************************************/
#include "../cpp/Robot.hpp"

#include "motion.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "encoder.h"
#include "global.h"
#include "log.h"
#include "motion_priv.h"
#include "motor.h"
#include "motor_positionCommand.h"
#include "motor_PID.h"
#include "motor_speedCommand.h"
#include "motor_stepCommand.h"
#include "path_manager.h"
#include "robot.h"
#include "robot_odometry.h"
#include "robot_slippage.h"

//#include "robot_trajectory.h"

MOTION_STATE RobotMotionState;

//nb of period since the beginning
static uint32 periodNb;

//semaphore for motion IT and Task
//static sem_t semMotionIT;

//mutex protecting the currently executed command
static pthread_mutex_t mtxMotionCommand;
static pthread_t thread;
//the currently executed command
static RobotCommand motionCommand;

//motion control task
void *motion_ITTask(void *p_arg);

void motion_InitTimer(int frequency);
void initPWM(void);
void signalEndOfTraj(void);

//declaration of some internal function :
void path_Init(void);
void path_TriggerWaypoint(TRAJ_STATE state);

long loopDelayInMillis;

void motion_configureAlphaPID(float p, float i, float d)
{
	pid_Config(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, p, i, d);
}
void motion_configureDeltaPID(float p, float i, float d)
{
	pid_Config(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, p, i, d);
}
void motion_configureLeftPID(float p, float i, float d)
{
	pid_Config(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, p, i, d);
}
void motion_configureRightPID(float p, float i, float d)
{
	pid_Config(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, p, i, d);
}

void motion_Init()
{
	//printf("motion_Init===========\n");
	int i, j;
	RobotMotionState = DISABLE_PID;
	periodNb = 0;

	initPWM();

	pid_Init();
	//motor initialisation
	for (i = 0; i < MAX_MOTION_CONTROL_TYPE_NUMBER; i++)
	{
		for (j = 0; j < MOTOR_PER_TYPE; j++)
		{
			initMotor(&motors[i][j]);
		}
	}

	encoder_Init();

	motion_FreeMotion();

	//timing semaphore : unlock the motion control task periodically
	//sem_init(&semMotionIT, 0, 0);

	//currently executed motion command protection mutex
	pthread_mutex_init(&mtxMotionCommand, NULL);

	//create motion control task
	if (pthread_create(&thread, NULL, motion_ITTask, NULL) < 0)
	{
		fprintf(stderr, "pthread_create error for thread 1\n");
		exit(1);
	}

	motion_InitTimer(DEFAULT_SAMPLING_FREQ);
	traj_Init();
	slippage_Init();

	//path manager initialisation
	path_Init();
}
void motion_Destroy()
{

}
void signalEndOfTraj()
{
#ifdef DEBUG_MOTION
	printf("motions.c : signalEndOfTraj %d\n", nextWaypoint.type);
#endif
	if (motionCommand.cmdType == POSITION_COMMAND)
	{
		motion_FreeMotion();
	}

	if (nextWaypoint.type == WP_END_OF_TRAJ)
	{
#ifdef DEBUG_MOTION
		printf("signalEndOfTraj path_TriggerWaypoint\n");
#endif
		path_TriggerWaypoint(TRAJ_OK);
	}

}

void motion_FreeMotion()
{
	RobotMotionState = FREE_MOTION;
	setPWM(0, 0);
	setPWM(0, 0);
	//printf("============> motion_FreeMotion setPWM(0, 0);\n");
}

void motion_AssistedHandling()
{
	motion_FreeMotion();
	RobotMotionState = ASSISTED_HANDLING;
}

void motion_DisablePID()
{
	motion_FreeMotion();
	RobotMotionState = DISABLE_PID;
	setPWM(0, 0);

}
void checkRobotCommand(RobotCommand *cmd)
{
	int cmdType = cmd->cmdType;
	if (cmdType != POSITION_COMMAND && cmdType != SPEED_COMMAND && cmdType != STEP_COMMAND)
	{
		printf("motion.c checkRobotCommand : ERROR: bad command type: %d\n", cmdType);
		exit(1);
	}
	int mcType = cmd->mcType;
	if (mcType != ALPHA_DELTA && mcType != LEFT_RIGHT)
	{
		printf("motion.c checkRobotCommand : ERROR: bad control type: %d\n", cmdType);
		exit(1);
	}
}
void loadCommand(RobotCommand *cmd)
{
	checkRobotCommand(cmd);
	switch (cmd->cmdType)
	{
	case POSITION_COMMAND:
		LoadPositionCommand(&cmd->cmd.posCmd[0], &motors[cmd->mcType][0], periodNb);
		LoadPositionCommand(&cmd->cmd.posCmd[1], &motors[cmd->mcType][1], periodNb);
		break;

	case SPEED_COMMAND:
		//if we are in same command mode, we get the next order and apply it
		//as the starting point of new trajectory
		if (motionCommand.cmdType == SPEED_COMMAND && motionCommand.mcType == cmd->mcType)
		{
			int32 pos0, pos1;
			GetSpeedOrder(&motionCommand.cmd.speedCmd[0], periodNb, &pos0);
			GetSpeedOrder(&motionCommand.cmd.speedCmd[1], periodNb, &pos1);

			LoadSpeedCommand(&cmd->cmd.speedCmd[0], pos0, periodNb);
			LoadSpeedCommand(&cmd->cmd.speedCmd[1], pos1, periodNb);
		}
		else
		{
			LoadSpeedCommand(&cmd->cmd.speedCmd[0], motors[cmd->mcType][0].lastPos, periodNb);
			LoadSpeedCommand(&cmd->cmd.speedCmd[1], motors[cmd->mcType][1].lastPos, periodNb);
		}
		break;

	default:
		break;
	};
}

void motion_SetCurrentCommand(RobotCommand *cmd)
{
	checkRobotCommand(cmd);
#ifdef DEBUG_MOTION
	printf("motion.c : motion_SetCurrentCommand  \n");
#endif
	RobotMotionState = DISABLE_PID;

	pthread_mutex_lock(&mtxMotionCommand);
#ifdef DEBUG_MOTION
	printf("motion.c : motion_SetCurrentCommand lock done commandType:%d controlType:%d\n",
			cmd->cmdType, cmd->mcType);
#endif
	loadCommand(cmd);
	motionCommand = *cmd;
	RobotMotionState = TRAJECTORY_RUNNING;
	pthread_mutex_unlock(&mtxMotionCommand);
#ifdef DEBUG_MOTION
	printf("motion.c : motion_SetCurrentCommand done\n");
#endif
}

//Motion control main loop
//implemented as an IT task, armed on a timer to provide
//a constant and precise period between computation
void *motion_ITTask(void *p_arg)
{
#ifdef LOG_PID_APPENDER
	pmx
	::Robot &logrobot = pmx::Robot::instance();
#endif

//static int32 left, right;
//static int32 alpha, delta;

	static int32 dLeft, dRight;
	static int32 dAlpha, dDelta;

//static int32 dLeft2, dRight2;
//static int32 dAlpha2, dDelta2;

	static int32 ord0, ord1;
	static BOOL fin0, fin1;
	static int32 pwm0, pwm1;
	static int32 pwm0b, pwm1b;
	static int32 error0, error1;

#ifdef DEBUG_MOTION
	printf("motion.c : motion_ITTask start\n");
#endif
	int stop = 0;
	while (!stop)
	{

		//sem_wait(&semMotionIT);

		long startTime = currentTimeInMillis();
#ifdef DEBUG_MOTION
		printf("motion.c : ------  updating state ---- time : %ld ms [%d]\n",
				currentTimeInMillis(), RobotMotionState);
#endif
		periodNb++;

		encoder_ReadSensor(&dLeft, &dRight, &dAlpha, &dDelta);

		odo_Integration(2 * dAlpha / (float) distEncoder, (float) dDelta);

		RobotPosition p = odo_GetPosition();

		//	printf("ROBOT POS: %f,%f theta:%f\n", p.x, p.y, p.theta);
		//send position
		//	if ((periodNb & 0x3F) == 0) {
//			pos_SendPosition();
//		}

		//update all motors
		updateMotor(&motors[LEFT_RIGHT][LEFT_MOTOR], dLeft);
		updateMotor(&motors[LEFT_RIGHT][RIGHT_MOTOR], dRight);
		updateMotor(&motors[ALPHA_DELTA][ALPHA_MOTOR], dAlpha);
		updateMotor(&motors[ALPHA_DELTA][DELTA_MOTOR], dDelta);
		double dSpeed0 = 0;
		double dSpeed1 = 0;
		//order and pwm computation
		switch (RobotMotionState)
		{
		case TRAJECTORY_RUNNING:
			//lock motionCommand
			pthread_mutex_lock(&mtxMotionCommand);

			if (motionCommand.mcType == LEFT_RIGHT)
			{
				dSpeed0 = dLeft;
				dSpeed1 = dRight;
			}
			else if (motionCommand.mcType == ALPHA_DELTA)
			{
				dSpeed0 = dAlpha * 2.0;
				dSpeed1 = dDelta;
			}
			else
			{
				printf("motion.c : motion_ITTask : motionCommand error: %d (%d %d %d)\n",
						motionCommand.mcType, LEFT_RIGHT, ALPHA_DELTA,

						MAX_MOTION_CONTROL_TYPE_NUMBER);
				exit(1);
			}

			//choose the right function to compute new order value
			switch (motionCommand.cmdType)
			{
			case POSITION_COMMAND:
#ifdef DEBUG_MOTION
				printf("motion.c :  POSITION_COMMAND\n");
#endif
				fin0 = GetPositionOrder(&motionCommand.cmd.posCmd[0], periodNb, &ord0);
				fin1 = GetPositionOrder(&motionCommand.cmd.posCmd[1], periodNb, &ord1);

				break;

			case SPEED_COMMAND:
#ifdef DEBUG_MOTION
				printf("motion.c :  SPEED_COMMAND\n");
#endif
				fin0 = GetSpeedOrder(&motionCommand.cmd.speedCmd[0], periodNb, &ord0);
				fin1 = GetSpeedOrder(&motionCommand.cmd.speedCmd[1], periodNb, &ord1);
				break;

			case STEP_COMMAND:
#ifdef DEBUG_MOTION
				printf("motion.c :  STEP_COMMAND\n");
#endif
				fin0 = GetStepOrder(&motionCommand.cmd.stepCmd[0], &ord0);
				fin1 = GetStepOrder(&motionCommand.cmd.stepCmd[1], &ord1);
				break;
			}

#ifdef LOG_PID

			log_status(currentTimeInMillis(), encoder_getLeftCounter(),
					encoder_getRightCounter(), robot_getLeftPower(),
					robot_getRightPower(), ord0, ord1,
					motors[motionCommand.mcType][0].lastPos,
					motors[motionCommand.mcType][1].lastPos, p.x, p.y, p.theta);
#endif

#ifdef LOG_SVG_APPENDER
			apf_svg_writePosition(p.x, p.y, p.theta);
#endif

			//compute pwm for first motor
			//pwm0 = pid_Compute(motors[motionCommand.mcType][0].PIDSys, ord0, motors[motionCommand.mcType][0].lastPos, dSpeed0);
			//test with Rcva pid
			error0 = ord0 - motors[motionCommand.mcType][0].lastPos;
			pwm0 = pid_ComputeRcva(motors[motionCommand.mcType][0].PIDSys, error0, dSpeed0);


			//compute pwm for second motor
			//pwm1 = pid_Compute(motors[motionCommand.mcType][1].PIDSys, ord1, motors[motionCommand.mcType][1].lastPos, dSpeed1);

			error1 = ord1 - motors[motionCommand.mcType][1].lastPos;
			pwm1 = pid_ComputeRcva(motors[motionCommand.mcType][1].PIDSys, error1, dSpeed1);

			//output pwm to motors
			if (motionCommand.mcType == LEFT_RIGHT)
			{
				//	printf("motion.c : LEFT_RIGHT mode, pid result : %d %d \n",	pwm0, pwm1);
				BOUND_INT(pwm0, MAX_PWM_VALUE);
				BOUND_INT(pwm1, MAX_PWM_VALUE);
				pwm0b = pwm0;
				pwm1b = pwm1;
				setPWM(pwm0b, pwm1b);
			}
			else if (motionCommand.mcType == ALPHA_DELTA)
			{
				// printf("motion.c : ALPHA_DELTA mode, pid result : %d %d \n", pwm0, pwm1);
				pwm0b = pwm1 - pwm0;
				BOUND_INT(pwm0b, MAX_PWM_VALUE);

				pwm1b = pwm1 + pwm0;
				BOUND_INT(pwm1b, MAX_PWM_VALUE);

				setPWM(pwm0b, pwm1b);
			}

#ifdef LOG_PID_APPENDER

			apf_log(currentTimeInMillis(), dSpeed0, dSpeed1, pwm0b, pwm1b, ord0, ord1,
					motors[motionCommand.mcType][0].lastPos,
					motors[motionCommand.mcType][1].lastPos, p.x, p.y, p.theta);

			logrobot.base().mlogger().debug() << "Motion.c p=" << periodNb << "\tdSpeed=" << dSpeed1
					<< "\tpwm=" << pwm1b << "\tord1=" << ord1 << utils::end;
#endif

			//test end of traj
			if (fin0 && fin1)
			{

#ifdef LOG_PID
				closeLog();
#endif

				//TODO à enlever pour les tests steporder
				signalEndOfTraj();
			}

			//unlock motionCommand
			pthread_mutex_unlock(&mtxMotionCommand);

			break;

		case ASSISTED_HANDLING:
		{
			//printf("motion_ITTask ASSISTED_HANDLING  \n");
			dSpeed0 = dLeft;
			dSpeed1 = dRight;
			//compute pwm for first motor
			//pwm0 = pid_Compute(motors[motionCommand.mcType][0].PIDSys, 0, motors[motionCommand.mcType][0].lastPos, dSpeed0);
			pwm0 = pid_ComputeRcva(motors[motionCommand.mcType][0].PIDSys, dLeft, dSpeed0);

			//compute pwm for second motor
			//pwm1 = pid_Compute(motors[motionCommand.mcType][1].PIDSys, 0, motors[motionCommand.mcType][1].lastPos, dSpeed1);
			pwm1 = pid_ComputeRcva(motors[motionCommand.mcType][1].PIDSys, dRight, dSpeed1);

			//printf("motion_ITTask ASSISTED_HANDLING  pwm0=%d pwm1=%d \n", pwm0, pwm1);
			//write pwm in registers
			setPWM(pwm0, pwm1);
			break;
		}

		case DISABLE_PID:
		{
			//printf("motion_ITTask DISABLE_PID  \n");
			break;
		}
		case FREE_MOTION:
		{
			//printf("motion_ITTask FREE_MOTION  \n");
			break;
		}
		default:
			break;
		};

		long stopTime = currentTimeInMillis();
		long duration = stopTime - startTime;
		if (duration < loopDelayInMillis && duration >= 0)
		{
			__useconds_t d = 1000 * (loopDelayInMillis - duration);
			usleep(d);
		}

	}
#ifdef DEBUG_MOTION
	printf("motion.c : motion_ITTask end exit()\n");
#endif

	/*
	 #ifdef LOG_PID
	 printf("closelog");
	 sleep(1);
	 closeLog();
	 #endif*/
//exit(2);
	return 0;
}

//Motion control main loop
//implemented as an IT handler armed on a timer to provide
//a constant and precise period between computation
void motion_InitTimer(int frequency)
{
	loopDelayInMillis = 1000 / frequency;
#ifdef DEBUG_MOTION
	printf("motion.c : motion_InitTimer with pause of %ld us\n", loopDelayInMillis);
#endif
}

void motion_StopTimer()
{
	setPWM(0, 0);
	setPWM(0, 0);
	//TODO kill timer	//stop = 1 ;
}

void initPWM()
{

}

void setPWM(int16 pwmLeft, int16 pwmRight)
{
#ifdef DEBUG_MOTION
	printf("motion.c setPWM : left %d  right %d\n", pwmLeft, pwmRight);
#endif
	robot_setMotorLeftSpeed(pwmLeft);
	robot_setMotorRightSpeed(pwmRight);
}

