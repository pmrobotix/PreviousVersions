#pragma config(Sensor, S1,     HTANG_LEFT,     sensorI2CCustom)
#pragma config(Sensor, S4,     HTANG_RIGHT,    sensorI2CCustom)
#pragma config(Sensor, S2,     TOUCH_SENSOR,    sensorTouch)
#pragma config(Sensor, S3,     SONAR,          sensorSONAR)
#pragma config(Motor,  motorA,          MOTOR_LEFT,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorC,          MOTOR_RIGHT,   tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,          MOTOR_PUSH,   tmotorNXT, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//
#pragma platform(NXT)

#ifndef _MOTION
#define _MOTION // prevent multiple compilations

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: ROBOTC PROJECT MAIN FILE DECLARATION
#ifndef PROJECT_MAIN_FILE
#define PROJECT_MAIN_FILE "cmotion.c" //!< enable compilation of task main() and various debug and test functions of this file
#endif
// END OF:  ROBOTC PROJECT MAIN FILE DECLARATION
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: HEADERS
#include "cmotion.h"
// END OF:  HEADERS
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: GLOBAL INSTANCES
MOTION_STATE RobotMotionState;
//the currently executed command
static RobotCommand motionCommand;
// END OF:  GLOBAL INSTANCES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: EXTERNAL SOURCES
#ifndef _ENCODER_EXTERN
#include "cencoderExtern.c"
#endif
#ifndef _ODOMETRY
#include "codometry.c"
#endif
#ifndef _ROBOT_TRAJECTORY
#include "crobotTrajectory.c"
#endif
#ifndef _PATH_MANAGER
#include "cpathManager.c"
#endif
#ifndef _ROBOT_SLIPPAGE
#include "crobotSlippage.c"
#endif

#include "cposition.c" //TODO CCH pos

// END OF:  EXTERNAL SOURCES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: LOCAL INSTANCES
//nb of period since the beginning
static long periodNb;
//semaphore for motion IT and Task
//static OS_EVENT *semMotionIT;
//mutex protecting the currently executed command
//static OS_EVENT *mtxMotionCommand;


static long dLeft = 0, dRight = 0;
static long dAlpha = 0, dDelta = 0;
static long dLeft2 = 0, dRight2 = 0;
static long dAlpha2 = 0, dDelta2 = 0;
static long ord0 = 0, ord1 = 0;
static bool fin0 = false, fin1 = false;
static long pwm0 = 0, pwm1 = 0;
static long pwm0Go = 0, pwm1Go = 0;

// END OF:  LOCAL INSTANCES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: CODES

void motion_Init()
{
	int i,j;
	periodNb = 0;
	dLeft2 = 0;
	dRight2 = 0;
	dAlpha2 = 0;
	dDelta2 = 0;

	//init led pour mesure tempo asserv
	//pd13_5 = 1;			// led 13_5 - > out

	initPWM();
	pid_Init();
	//motor initialisation
	for(i=0; i<(int)MAX_MOTION_CONTROL_TYPE_NUMBER; i++)
	{
		for(j=0; j<MOTOR_PER_TYPE; j++)
		{
			initMotor(&motors[i][j]);
		}
	}

	//encoder initialisation
	motion_CalibrationInit();

	encoder_Init();
	externEncoder_Init();
	//mouse_Init();


	motion_FreeMotion();

	//timing semaphore : unlock the motion control task periodically
	//semMotionIT = OSSemCreate(0);
	//if(!semMotionIT)
	//{
	//	LOG_ERROR("Unable to create semMotionIT");
	//}

	//currently executed motion command protection mutex
	//mtxMotionCommand = OSMutexCreate(MUTEX_MOTION_COMMAND_PRIO, &OSLastError);
	//LOG_TEST_OS_ERROR(OSLastError);

	//create motion control task
	//OSLastError = OSTaskCreate(motion_ITTask, NULL, getMotionTopOfStack(), MOTION_PRIO);
	//LOG_TEST_OS_ERROR(OSLastError);


	motion_InitTimer(DEFAULT_SAMPLING_PERIOD_Ms);
	traj_Init();
	slippage_Init();

	//path manager initialisation
	path_Init();

	//start Motion task
	#if(LEVEL_MOTION <= DEBUG)
	writeDebugStreamLine("cmotion.c : motion_Init : nxt_startMotionITTask !!");
	#endif
	nxt_startMotionITTask(5, valueSample * 1000);

	#if(LEVEL_PM_GRAPH_PID <= INFO)
	writeDebugStreamLine("A;periodNb;ord0;error0;pwm0;dSpeed0;pwm0Go;x;y;theta;");
	writeDebugStreamLine("D;periodNb;ord1;error1;pwm1;dSpeed1;pwm1Go;x;y;theta;");
	#endif

}


void signalEndOfTraj()
{
	#if(LEVEL_MOTION <= DEBUG)
	writeDebugStreamLine("cmotion.c DEBUG signalEndOfTraj");
	#endif
	if(motionCommand.cmdType == POSITION_COMMAND)
	{
		motion_FreeMotion();
	}

	if(nextWaypoint.type == WP_END_OF_TRAJ)
	{
		path_TriggerWaypoint(TRAJ_OK);
	}
}

void resetAllPIDErrors(void)
{
	pid_ResetError(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys);
	pid_ResetError(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys);
	pid_ResetError(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys);
	pid_ResetError(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys);
}

void motion_FreeMotion()
{
	#if(LEVEL_MOTION <= DEBUG)
	writeDebugStreamLine("cmotion.c DEBUG motion_FreeMotion");
	#endif
	resetAllPIDErrors();
	RobotMotionState = FREE_MOTION;
	setPWM(0,0);
	//OSTimeDly(1);
	Sleep(20);
	RobotMotionState = FREE_MOTION;
	setPWM(0,0);
	////OSTimeDly(1);
	Sleep(20);
	RobotMotionState = FREE_MOTION;
	setPWM(0,0);
	resetAllPIDErrors();
}

void motion_AssistedHandling()
{
	motion_FreeMotion();
}

void motion_DisablePID()
{
	motion_FreeMotion();
	RobotMotionState = DISABLE_PID;
	setPWM(0,0);
	//OSTimeDly(1);
	Sleep(20);
	RobotMotionState = DISABLE_PID;
	setPWM(0,0);
	resetAllPIDErrors();
}

void loadCommand(RobotCommand *cmd)
{
	switch(cmd->cmdType)
	{
	case POSITION_COMMAND:
		LoadPositionCommand(&cmd->cmd.posCmd[0], &motors[cmd->mcType][0], periodNb);
		LoadPositionCommand(&cmd->cmd.posCmd[1], &motors[cmd->mcType][1], periodNb);
		break;

	case SPEED_COMMAND:
		//if we are in same command mode, we get the next order and apply it
		//as the starting point of new trajectory
		if(motionCommand.cmdType == SPEED_COMMAND && motionCommand.mcType == cmd->mcType)
		{
			long pos0, pos1;
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
	};
}

void motion_SetCurrentCommand(RobotCommand *cmd)
{
	RobotMotionState = DISABLE_PID;
	//OSMutexPend(mtxMotionCommand, 0, &OSLastError);
	//LOG_TEST_OS_ERROR(OSLastError);

		loadCommand(cmd);
		motionCommand = *cmd;
		RobotMotionState = TRAJECTORY_RUNNING;

	//OSLastError = OSMutexPost(mtxMotionCommand);
	//LOG_TEST_OS_ERROR(OSLastError);
}

void setPWM(int pwmLeft, int pwmRight)
{
	BOUND_INT(pwmLeft, MAX_PWM_VALUE);
	BOUND_INT(pwmRight, MAX_PWM_VALUE);

	nxt_ApplyMotorLeft(pwmLeft);
	nxt_ApplyMotorRight(pwmRight);
}

void initPWM()
{
	//Active speed control on each nxt motor
	nxt_InitPIDSpeedCtrl();
}

//Motion control main loop
//implemented as an IT task, armed on a timer to provide
//a constant and precise period between computation
void motion_Compute() //void *p_arg
{

	periodNb++;

	#if(LEVEL_MOTION <= DEBUG)
	writeDebugStreamLine("cmotion.c : motion_Compute %d", periodNb);
	#endif

	//#ifdef USE_EXTERN_ENCODER
	#if(USE_EXTERN_ENCODER)
	//if(odometrySensorUsed == MOTOR_ENCODER)
	//{
	//	#ifdef USE_ENCODER
	//		encoder_ReadSensor(&dLeft, &dRight, &dAlpha, &dDelta);
	//	#else//NO USE_ENCODER
	//		dLeft = 0; dRight = 0;
	//		dAlpha = 0; dDelta = 0;
	//	#endif//END USE_ENCODER
	//}
	//else
	//{
		externEncoder_ReadSensor(&dLeft, &dRight, &dAlpha, &dDelta);

		//#ifdef USE_ENCODER
		#if(USE_ENCODER)
		encoder_ReadSensor(&dLeft2, &dRight2, &dAlpha2, &dDelta2);

			//#ifdef SLIPPAGE_DETECTION
			#if(SLIPPAGE_DETECTION)
			slippage_Update(LEFT_MOTOR, dLeft2, dLeft, slippageThreshold);
			slippage_Update(RIGHT_MOTOR, dRight2, dRight, slippageThreshold);
			#endif//SLIPPAGE_DETECTION

		#endif//END USE_ENCODER
	//}

	#else//NOT USE_EXTERN_ENCODER
		#if(LEVEL_MOTION <= DEBUG)
		writeDebugStreamLine("cmotion.c : Compute : NOT USE_EXTERN_ENCODER");
		#endif
		//#ifdef USE_ENCODER
		#if(USE_ENCODER)
			encoder_ReadSensor(&dLeft, &dRight, &dAlpha, &dDelta);
		#else//NOT USE_ENCODER
			dLeft = 0; dRight = 0;
			dAlpha = 0; dDelta = 0;
		#endif//END USE_ENCODER
	#endif//END USE_EXTERN_ENCODER

	#if(LEVEL_MOTION <= DEBUG)
	writeDebugStreamLine("cmotion.c : internal: dLeft %.2f; dRight %.2f; dAlpha %.2f; dDelta %.2f", dLeft, dRight, dAlpha, dDelta);
	writeDebugStreamLine("cmotion.c : external: dLeft %.2f; dRight %.2f; dAlpha %.2f; dDelta %.2f", dLeft2, dRight2, dAlpha2, dDelta2);
	writeDebugStreamLine("cmotion.c : dTheta %f", 2.0*dAlpha/(float)distEncoder);
	#endif

	odo_Integration(2.0*dAlpha/(float)distEncoder, (float)dDelta);



	////send position //TODO CCH pos
	//if((periodNb&0x3F) == 0)
	//{
	//	pos_SendPosition(); //TODO enregistrer la position du robot
	//}

	//update all motors
	updateMotor(&motors[LEFT_RIGHT][LEFT_MOTOR], dLeft);
	updateMotor(&motors[LEFT_RIGHT][RIGHT_MOTOR], dRight);
	updateMotor(&motors[ALPHA_DELTA][ALPHA_MOTOR], dAlpha);
	updateMotor(&motors[ALPHA_DELTA][DELTA_MOTOR], dDelta);

	#if(LEVEL_MOTION <= DEBUG)
	writeDebugStreamLine("cmotion.c : Compute updtMotor dLeft=%d, dRight=%d, dAlpha=%d, dDelta=%d",dLeft, dRight, dAlpha, dDelta);
	#endif

	long dSpeed0 = 0;
	long dSpeed1 = 0;
	//if(RobotMotionState!=DISABLE_PID) //TODO necessaire ?
 // {
	//order and pwm computation
	switch(RobotMotionState)
	{
	case TRAJECTORY_RUNNING:
		{
		//lock motionCommand
		//OSMutexPend(mtxMotionCommand, 0, &OSLastError);
		//LOG_TEST_OS_ERROR(OSLastError);

			//choose the right function to compute new order value
			switch(motionCommand.cmdType)
			{
				case POSITION_COMMAND:
					#if(LEVEL_MOTION <= DEBUG)
					writeDebugStreamLine("cmotion.c : Compute TRAJECTORY_RUNNING : cmdType POSITION_COMMAND");
					#endif
					fin0 = GetPositionOrder(&motionCommand.cmd.posCmd[0], periodNb, &ord0);
					fin1 = GetPositionOrder(&motionCommand.cmd.posCmd[1], periodNb, &ord1);
					break;

				case SPEED_COMMAND:
					#if(LEVEL_MOTION <= DEBUG)
					writeDebugStreamLine("cmotion.c : Compute TRAJECTORY_RUNNING : cmdType SPEED_COMMAND");
					#endif
					fin0 = GetSpeedOrder(&motionCommand.cmd.speedCmd[0], periodNb, &ord0);
					fin1 = GetSpeedOrder(&motionCommand.cmd.speedCmd[1], periodNb, &ord1);
					break;

				case STEP_COMMAND:
					#if(LEVEL_MOTION <= DEBUG)
					writeDebugStreamLine("cmotion.c : Compute TRAJECTORY_RUNNING : cmdType STEP_COMMAND : ord0=%d, ord1=%d", ord0, ord1);
					#endif
					fin0 = GetStepOrder(&motionCommand.cmd.stepCmd[0], &ord0);
					fin1 = GetStepOrder(&motionCommand.cmd.stepCmd[1], &ord1);
					break;
			}

			long error0 = ord0 - motors[motionCommand.mcType][0].lastPos;
			long error1 = ord1 - motors[motionCommand.mcType][1].lastPos;

//#if(LEVEL_MOTION <= INFO)
//writeDebugStreamLine("error0=%d ; ord0=%d ; lastPos0=%d", error0, ord0, motors[motionCommand.mcType][0].lastPos);
//writeDebugStreamLine("error1=%d ; ord1=%d ; lastPos1=%d", error1, ord1, motors[motionCommand.mcType][1].lastPos);
//#endif
			//compute delta speed
			if(motionCommand.mcType == LEFT_RIGHT)
			{
				dSpeed0 = dLeft;
				dSpeed1 = dRight;
			}
			else if(motionCommand.mcType == ALPHA_DELTA)
			{
				dSpeed0 = dAlpha * 2; // *2 to avoid average on alpha //TODO a verifier
				dSpeed1 = dDelta;
			}

			//compute pwm for first motor
			//pwm0 = pid_Compute(motors[motionCommand.mcType][0].PIDSys, error0);
			pwm0 = pid_ComputeRcva(motors[motionCommand.mcType][0].PIDSys, error0, dSpeed0);

			//compute pwm for second motor
			//pwm1 = pid_Compute(motors[motionCommand.mcType][1].PIDSys, error1);
			pwm1 = pid_ComputeRcva(motors[motionCommand.mcType][1].PIDSys, error1, dSpeed1);

			#if(LEVEL_PM_GRAPH_PID <= INFO)
			float x1, y1, theta1;
			odo_GetPositionXYTheta(&x1, &y1, &theta1);
			//writeDebugStreamLine("cmotion.c : x=%f  y=%f  theta=%f",x1, y1, theta1);
			#endif

			//output pwm to motors
			if(motionCommand.mcType == LEFT_RIGHT)
			{
				pwm0Go = pwm0;
				pwm1Go = pwm1;

				#if(LEVEL_PM_GRAPH_PID <= INFO)
				writeDebugStreamLine("A;%d;%d;%d;%d;%d;%d;%f;%f;%f;", periodNb, ord0, error0, pwm0, dSpeed0, pwm0Go, x1, y1, theta1);
				writeDebugStreamLine("D;%d;%d;%d;%d;%d;%d;%f;%f;%f;", periodNb, ord1, error1, pwm1, dSpeed1, pwm1Go, x1, y1, theta1);
				#endif
			}
			else if(motionCommand.mcType == ALPHA_DELTA)
			{
				pwm0Go = pwm1 - pwm0;
				pwm1Go = pwm1 + pwm0;

				#if(LEVEL_PM_GRAPH_PID <= INFO)
				writeDebugStreamLine("A;%d;%d;%d;%d;%d;%d;%f;%f;%f;", periodNb, ord0, error0, pwm0, dSpeed0, pwm0Go, x1, y1, theta1);
				writeDebugStreamLine("D;%d;%d;%d;%d;%d;%d;%f;%f;%f;", periodNb, ord1, error1, pwm1, dSpeed1, pwm1Go, x1, y1, theta1);
				#endif
			}

			//BOUND_INT(pwm0Go, 70);
			//BOUND_INT(pwm1Go, 70);

			//Apply pwm on Motors
			setPWM(pwm0Go, pwm1Go);

			////if required send both errors
			//if(isSendErrorsEnabled)
			//{
			//	SendMotorErrors(ord0 - motors[motionCommand.mcType][0].lastPos,
			//					ord1 - motors[motionCommand.mcType][1].lastPos);
			//}

			//test end of traj
			//int diffPos=

			if(fin0 && fin1)
			{
				#if(LEVEL_MOTION <= DEBUG)
				writeDebugStreamLine("cmotion.c : signalEndOfTraj");
				#endif
				signalEndOfTraj();
			}

		//unlock motionCommand
		//OSLastError = OSMutexPost(mtxMotionCommand);
		//LOG_TEST_OS_ERROR(OSLastError);
		break;
		}
	case ASSISTED_HANDLING:
		{
			#if(LEVEL_MOTION <= DEBUG)
			writeDebugStreamLine("cmotion.c : ASSISTED_HANDLING");
			#endif

			dSpeed0 = dLeft;
			dSpeed1 = dRight;
			//compute pwm for first motor
			//pwm0 = pid_Compute(motors[motionCommand.mcType][0].PIDSys, dLeft);
			pwm0 = pid_ComputeRcva(motors[motionCommand.mcType][0].PIDSys, dLeft, dSpeed0);

			//compute pwm for second motor
			//pwm1 = pid_Compute(motors[motionCommand.mcType][1].PIDSys, dRight);
			pwm1 = pid_ComputeRcva(motors[motionCommand.mcType][1].PIDSys, dRight, dSpeed1);

			//write pwm in registers
			setPWM(pwm0, pwm1);
		}
		break;


	case DISABLE_PID:
	case FREE_MOTION:
	default:
	{
		#if(LEVEL_MOTION <= DEBUG)
		writeDebugStreamLine("cmotion.c : DISABLE_PID or FREE_MOTION");
		#endif
		break;
	}
	};
}


//Motion control main loop
//implemented as an IT handler armed on a timer to provide
//a constant and precise period between computation
void motion_InitTimer(int periodMs)
{
	//ta0mr = 0x40;   // XX0X XX00
					// |||| ||||
					// |||| |||+- must always be 0 in timer mode
					// |||| ||+-- must always be 0 in timer mode
					// |||| |+--- 0: pulse is not output at pin TA1out
					// |||| |     1: pulse is output at pin TA1out
					// |||| |        TA0out is automatically  output
					// |||| +---- 0: gate function: timer counts only
					// ||||          when TA0in is held "L"
					// ||||       1: gate function: timer counts only
					// ||||          when TA0in is held "H"
					// |||+------ 0: gate function not available
					// |||        1: gate function available
					// ||+------- must always be 0 in timer mode
					// |+-------- count source select bits:
					// +--------- count source select bits:
					//            00:  f1
					//            01:  f8
					//            10:  f2n
					//            11:  fc32


	if (valueSample == 0.0)
		motion_SetSamplingFrequency(periodMs);

	//// level 6 priority: larger number ==> higher priority
	//ta0ic = 0x06;

	//asm("\tNOP");
	//asm("\tNOP");

	//tabsr |= 0x01;          //  start timer A0
}

void motion_StopTimer()
{
	//tabsr &= 0xFE;          //  stop timer A0
	nxt_stopMotionITTask();
	setPWM(0, 0);

}

// END OF:  CODES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: TESTS

#if(PROJECT_MAIN_FILE == "cmotion.c")
	//-----------------------------------------------------------------------------------------------------------------------------
	//! @brief     this task tests the functions
	//! @return    none
	//! @author    cho
	//! @note
	//! @todo
	//-----------------------------------------------------------------------------------------------------------------------------
	task main()
	{

		//static RobotCommand nextCommand;
		//float dist = 1.000; // in meters
		float x1, y1, theta1;


		motion_Init();

		//dist=323298, VMax=23691, Accel=237, Decel=237
		//dist=-147078, VMax=23691, Accel=237, Decel=237

		//0.0000003377


		 writeDebugStreamLine("distEncoderMeter %f  valueVTops %f", distEncoderMeter, valueVTops);
		float val1 = PI / 2.0 * distEncoderMeter / 2.0;
	  float val2 = -PI / 2.0 * distEncoderMeter / 2.0;

		float va10 = val1 / valueVTops;
		float va20 = val2 / valueVTops;

		float test1 = convertDistTovTops(3.14 / 2.0 * distEncoderMeter / 2.0);
		float test2 = convertDistTovTops(-3.14 / 2.0 * distEncoderMeter / 2.0);
	 writeDebugStreamLine("%f", 1.0 / valueVTops);
	 writeDebugStreamLine("%f", PI);
	 writeDebugStreamLine("%f", -PI);
	 writeDebugStreamLine("%f", PI / 2.0);
	 writeDebugStreamLine("%f", -PI / 2.0);
	 writeDebugStreamLine("%f", -(PI / 2.0));
	 writeDebugStreamLine("%d", test1);
	 writeDebugStreamLine("%d", test2);





		//case TRAJ_SETPWM:
		//motion_DisablePID();
		//setPWM(10, 10);
		//Sleep(1000);

		////case TRAJ_FREEMOTION:
		//motion_FreeMotion();
		//path_CancelTrajectory();
		//Sleep(1000);

		//case TRAJ_LINE:
		//motion_Line(&motionCommand, dist);
		//path_LaunchTrajectory(&motionCommand);
		//TRAJ_STATE ts = path_WaitEndOfTrajectory();

		/* // Do not use, default values
		pid_ConfigKP(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 800);
		pid_ConfigKI(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 400);
		pid_ConfigDPeriod(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 3);
		pid_ConfigKP(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 500);
		pid_ConfigKI(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 500);
		pid_ConfigDPeriod(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 3);

		pid_ConfigKP(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, 75);
		pid_ConfigKI(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, 50);
		pid_ConfigKP(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, 75);
		pid_ConfigKI(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, 5);
		pid_ConfigKD(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, 50);
		motors_ConfigAllIMax(90000);
		*/

		pid_ConfigKP(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 800);
		pid_ConfigKI(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 400);
		pid_ConfigDPeriod(motors[ALPHA_DELTA][ALPHA_MOTOR].PIDSys, 3);
		pid_ConfigKP(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 10);
		pid_ConfigKI(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 0);
		pid_ConfigDPeriod(motors[ALPHA_DELTA][DELTA_MOTOR].PIDSys, 3);
		/*
		pid_ConfigKP(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, 1);
		pid_ConfigKI(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[LEFT_RIGHT][LEFT_MOTOR].PIDSys, 0);
		pid_ConfigKP(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, 1);
		pid_ConfigKI(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, 0);
		pid_ConfigKD(motors[LEFT_RIGHT][RIGHT_MOTOR].PIDSys, 0);
*/
		motors_ConfigAllIMax(90000);

		odo_GetPositionXYTheta(&x1, &y1, &theta1);
		#if(LEVEL_MOTION <= DEBUG)
		writeDebugStreamLine("x1=%f  y1=%f  theta1=%f",x1, y1, theta1);
		#endif

		//motion_SpeedControlLR(&motionCommand, float spLeft, float distLeft, float accLeft, float spRight, float distRight, float accRight);
		//motion_SpeedControlLR(&motionCommand, msg->data.floats[0], 1.0f, 0.2f, msg->data.floats[1], 1.0f, 0.2f);
		//motion_SpeedControlLR(&motionCommand, 0.3, 0.5, 0.3, 0.3, 0.5, 0.3);
		//motion_SpeedControlAD(

		motion_SetSamplingFrequency(100);

		motion_SetDefaultSpeed(0.01);
		motion_SetDefaultAccel(0.02);
		motion_SetDefaultDecel(0.02);


		motion_Rotate(&motionCommand, 0);
		path_LaunchTrajectory(&motionCommand);
		TRAJ_STATE ts = path_WaitEndOfTrajectory();
		while(1==1){}
		/*
		motion_Line(&motionCommand, dist);
		path_LaunchTrajectory(&motionCommand);
		TRAJ_STATE ts = path_WaitEndOfTrajectory();
		*/
/*
		motion_Line(&motionCommand, dist);
		path_LaunchTrajectory(&motionCommand);
		ts = path_WaitEndOfTrajectory();

		motion_Line(&motionCommand, -dist);
		path_LaunchTrajectory(&motionCommand);
		ts = path_WaitEndOfTrajectory();

		motion_Line(&motionCommand, dist);
		path_LaunchTrajectory(&motionCommand);
		ts = path_WaitEndOfTrajectory();
*/
		//motion_LineSpeed(&motionCommand, -0.50, 0.8);

		//motion_StepOrderAD(&motionCommand, 0,	(1000*VTOPS_PER_TICKS));
	  //motion_StepOrderLR(&motionCommand, (1000*VTOPS_PER_TICKS), (1000*VTOPS_PER_TICKS));

		//path_LaunchTrajectory(&motionCommand);
		//TRAJ_STATE ts = path_WaitEndOfTrajectory();



		//case TRAJ_ASSISTEDHANDLING:
		//motion_AssistedHandling();

		//case TRAJ_SETVMAX:
		//motion_SetDefaultSpeed(msg->data.floats[0]);
		//break;

		//case TRAJ_SETACCEL:
		//motion_SetDefaultAccel(msg->data.floats[0]);
		//break;

		//case TRAJ_SETDECEL:
		//motion_SetDefaultDecel(msg->data.floats[0]);
		//break;

		odo_GetPositionXYTheta(&x1, &y1, &theta1);
		//#if(LEVEL_MOTION <= INFO)
		writeDebugStreamLine("x1=%f  y1=%f  theta1=%f",x1, y1, theta1);
		writeDebugStreamLine("cmotion.c : END OF NXT !!!!");
		//#endif


		motion_StopTimer();


	}
#endif // (PROJECT_MAIN_FILE)

// END OF:  TESTS
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif