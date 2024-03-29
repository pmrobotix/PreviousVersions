/*
 * ccbase.c
 *
 *  Created on: Apr 27, 2014
 *      Author: maillard
 */
#include "ccbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "robot.h"
#include "ia.h"
#include "motion.h"
#include "encoder.h"
#include "robot_odometry.h"
#include "log.h"
#include "global.h"
#include "path_manager.h"
boolean ignoreCollision = FALSE;
boolean collision = FALSE;
int matchColor = 0; //0=default color of the match

void launchAndWait(RobotCommand* cmd)
{
	checkRobotCommand(cmd);
	path_LaunchTrajectory(cmd);

	path_WaitEndOfTrajectory();
	//printf("path_WaitEndOfTrajectory returned : %d : %d\n", result, TRAJ_OK);
	free(cmd);
}

// if distance <0, move backward
void cc_move(float distanceInMM)
{
	RobotCommand* cmd = (RobotCommand*) malloc(sizeof(RobotCommand));
	float meters = distanceInMM / 1000.0f;
	motion_Line(cmd, meters);
	launchAndWait(cmd);
}

void cc_moveForwardTo(float xMM, float yMM)
{
	if (matchColor != 0)
	{
		yMM = -yMM;
	}
	float dx = xMM - cc_getX();
	float dy = yMM - cc_getY();
	float aRadian = atan2(dy, dx);
	cc_rotateTo((aRadian * 180.0f) / M_PI);
	float dist = sqrt(dx * dx + dy * dy);
	cc_move(dist);
}
void cc_moveForwardAndRotateTo(float xMM, float yMM, float thetaInDegree)
{
	if (matchColor != 0)
	{
		thetaInDegree = -thetaInDegree;
	}

	cc_moveForwardTo(xMM, yMM);
	cc_rotateTo(thetaInDegree);
}
void cc_moveBackwardTo(float xMM, float yMM)
{
	if (matchColor != 0)
	{
		yMM = -yMM;
	}

	float dx = xMM - cc_getX();
	float dy = yMM - cc_getY();
	float aRadian = atan2(dy, dx);
	cc_rotateTo(((M_PI + aRadian) * 180.0f) / M_PI);
	float dist = sqrt(dx * dx + dy * dy);
	cc_move(-dist);
}
void cc_moveBackwardAndRotateTo(float xMM, float yMM, float thetaInDegree)
{
	if (matchColor != 0)
	{
		thetaInDegree = -thetaInDegree;
	}
	cc_moveBackwardTo(xMM, yMM);
	cc_rotateTo(thetaInDegree);
}
void cc_rotateAbs(float degrees)
{
	RobotCommand* cmd = (RobotCommand*) malloc(sizeof(RobotCommand));
	float rad = (degrees * M_PI) / 180.0f;
	motion_Rotate(cmd, rad);
	launchAndWait(cmd);
}
void cc_rotateLeft(float degrees)
{
	cc_rotateAbs(degrees);
}
void cc_rotateRight(float degrees)
{
	cc_rotateAbs(-degrees);
}

void cc_rotateTo(float thetaInDegree)
{
	int c=ignoreCollision;
	ignoreCollision = TRUE;
	
	float currentThetaInDegree = cc_getThetaInDegree();
	
	float delta = thetaInDegree - currentThetaInDegree;

	float turn = ((int) (delta * 1000.0f) % 360000) / 1000.0f;

	cc_rotateLeft(turn);
	
	ignoreCollision=c;
}

// position x,x in mm
float cc_getX()
{
	RobotPosition p = odo_GetPosition();
	return p.x * 1000.0f;
}
float cc_getY()
{
	RobotPosition p = odo_GetPosition();
	return p.y * 1000.0f;
}
// angle in radian
float cc_getTheta()
{
	RobotPosition p = odo_GetPosition();
	return p.theta;
}
// angle in degrees
float cc_getThetaInDegree()
{
	RobotPosition p = odo_GetPosition();
	return (p.theta * 180.0f) / M_PI;
}

void cc_setIgnoreCollision(boolean b)
{
	ignoreCollision = b;
}

boolean cc_collisionOccured()
{
//TODO cch
//if(!ignoreCollision)
//		path_CollisionOnTrajectory();
	return collision;
}

void cc_setMatchColor(int color)
{
	matchColor = color;
}
int cc_getMatchColor()
{
	return matchColor;
}

void cc_setCollisionOccured(boolean b)
{
	collision = b;
}
void cc_setMirrorCoordinates(boolean b)
{
//TODO a supprimer
}
void cc_goToZone(const char *zoneName)
{
	ZONE* z = ia_getZone(zoneName);
	printf("%s (line %d) : goToZone %s\n", __FUNCTION__, __LINE__, zoneName);
	if (z == NULL)
	{
		printf("%s %d : unable to get zone %s\n", __FUNCTION__, __LINE__, zoneName);
	}
	else
	{
		ZONE *zCurrent = ia_getNearestZoneFrom(cc_getX(), cc_getY());

		ZONE_PATH *path = ia_getZonePath(zCurrent, z);
		printf("%s (line %d) : goToZone FROM %s TO %s using path (%f,%f)\n",
				__FUNCTION__,
				__LINE__, zCurrent->name, z->name, path->x, path->y);
		cc_moveForwardTo(path->x, path->y);
		// Move to destination
		cc_moveForwardAndRotateTo(z->startX, z->startY, z->startAngle);
	}
}
