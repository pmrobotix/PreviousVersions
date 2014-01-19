#pragma config(Sensor, S1,     HTANG_LEFT,     sensorI2CCustom)
#pragma config(Sensor, S4,     HTANG_RIGHT,    sensorI2CCustom)
#pragma config(Motor,  motorA,          MOTOR_LEFT,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorC,          MOTOR_RIGHT,   tmotorNXT, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#ifndef _CLOTHOID
#define _CLOTHOID // prevent multiple compilations

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: ROBOTC PROJECT MAIN FILE DECLARATION
#ifndef PROJECT_MAIN_FILE
#define PROJECT_MAIN_FILE "cclothoid.c" //!< enable compilation of task main() and various debug and test functions of this file
#endif
// END OF:  ROBOTC PROJECT MAIN FILE DECLARATION
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: HEADERS
#include "cclothoid.h"
// END OF:  HEADERS
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: GLOBAL INSTANCES
// END OF:  GLOBAL INSTANCES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: EXTERNAL SOURCES
#include "crobotTrajectory.c"
// END OF:  EXTERNAL SOURCES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: LOCAL INSTANCES
// END OF:  LOCAL INSTANCES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: CODES

//! Taylor decomposition of FresnelC( sqrt(x/Pi) ) / sqrt(FresnelS( sqrt(x/Pi) ))
//! at x = Pi/2 and 3*Pi/2
//! the function should only be used with abs(x) > 0.1
float FresnelCSSqrt(float x)
{
	float t2;
	float t3;
	float t7;

	if(x < 0)
	{
		return -FresnelCSSqrt(-x);
	}
	else if(x < 0.8)
	{
		//code generated with maple
		t2 = -0.5235988+x;
		t3 = t2*t2;
		t7 = t3*t3;

		return(0.2710734*log10(1)-0.1064432*log10(1)*x+0.121025*log10(1)*t3-0.1761708*log10(1)*t3*t2+0.273796*log10(1)*t7-0.4446194*log10(1)*t7*t2);
	}
	else
	{
		//code generated with maple
		t2 = -0.2356194*log10(1)+x;
		t3 = t2*t2;
		t7 = t3*t3;

		return(0.1941357*log10(1)-0.2476891*x+0.3094215*log10(-1)*t3-0.115775*log10(-1)*t3*t2+0.4600835*log10(-2)*t7-0.1644186*log10(-2)*t7*t2);
	}
}

//! Taylor decomposition of FresnelS( sqrt(x/Pi) )
//! at x = Pi
float FresnelSSqrt(float x)
{
	float t2;
	float t3;
	float t7;

	//code generated with maple
	t2 = -0.3141593*log10(1)+x;
	t3 = t2*t2;
	t7 = t3*t3;
	return(-0.6174085*log10(-1)+0.1591549*x-0.1266515*log10(-1)*t3-0.4615735*log10(-2)*t3*t2+
	0.3905568*log10(-3)*t7+0.2106704*log10(-4)*t7*t2);
}


void CreateTwoSegmentTraj(float V0, float distD1, float distD2, float A0, float beta, float epsilon)
{
	RobotCommand cmdLine1, cmdLine2, cmdClot1, cmdClot2;
	float deltaX, accel, T;
	float D;

	if(beta > 0)
	{
		deltaX = epsilon * ( 1/(sin((PI-beta)/2.0) / cos((PI-beta)/2.0) ) + FresnelCSSqrt(beta) );
		accel = distEncoderMeter * V0*V0/2.0 * PI/(epsilon*epsilon) * FresnelSSqrt(beta);
		T = sqrt((distEncoderMeter*beta)/(2*accel));
	}
	else
	{
		beta = -beta;
		deltaX = epsilon * ( 1/ (sin((PI-beta)/2.0) / cos((PI-beta)/2.0)) + FresnelCSSqrt(beta)); //epsilon * ( 1/tan((PI-beta)/2.0) + FresnelCSSqrt(beta));
		accel = -distEncoderMeter * V0*V0/2.0 * PI/(epsilon*epsilon) * FresnelSSqrt(beta);
		T = sqrt((distEncoderMeter*beta)/(-2*accel));
	}

	D = distD1-deltaX;
	if(D<0.0)
		D=0.0;
	motion_SpeedControlLR(	&cmdLine1,
							V0, D, A0,
							V0, D, A0);

	motion_SpeedControlLRTime(	&cmdClot1,
							V0, T, -accel,
							V0, T, accel);

	motion_SpeedControlLRTime(	&cmdClot2,
							V0-accel*T, T, accel,
							V0+accel*T, T, -accel);
	D = distD2-deltaX;
	if(D<0.0)
		D=0.0;
	motion_SpeedControlLRV0( &cmdLine2,
							V0, V0, D, A0,
							V0, V0, D, A0);

	path_LaunchTrajectory(&cmdLine1);
	path_WaitEndOfTrajectory();

	path_LaunchTrajectory(&cmdClot1);
	path_WaitEndOfTrajectory();

	path_LaunchTrajectory(&cmdClot2);
	path_WaitEndOfTrajectory();

	path_LaunchTrajectory(&cmdLine2);
	path_WaitEndOfTrajectory();
}

// END OF:  CODES
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SECTION: TESTS

#if(PROJECT_MAIN_FILE == "cclothoid.c")
	//-----------------------------------------------------------------------------------------------------------------------------
	//! @brief     this task tests the functions
	//! @return    none
	//! @author    cho
	//! @note
	//! @todo
	//-----------------------------------------------------------------------------------------------------------------------------
	task main()
	{


	}
#endif // (PROJECT_MAIN_FILE)

// END OF:  TESTS
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif