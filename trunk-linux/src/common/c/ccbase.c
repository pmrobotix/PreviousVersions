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

int matchColor = 0; //0=default color of the match (yellow)
boolean ignoreFrontCollision = TRUE;
boolean ignoreRearCollision = TRUE;

// en m/s (d'apres Cho)
float cc_motion_GetDefaultSpeed() {
	return defaultVmax;
}
float cc_motion_GetDefaultAccel() {
	return defaultAcc;
}
float cc_motion_GetDefaultDecel() {
	return defaultDec;
}

float cc_motion_GetDefaultSpeedForRotation() {
	return rotdefaultVmax;
}
float cc_motion_GetDefaultAccelForRotation() {
	return rotdefaultAcc;
}
float cc_motion_GetDefaultDecelForRotation() {
	return rotdefaultDec;
}


float cc_getRelativeX(float x) {
	if (cc_getMatchColor() != 0) {
		return 3000 - x;
	}
	return x;
}
float cc_getRelativeAngle(float degrees) {
	if (cc_getMatchColor() != 0) {
		return 180 - degrees;
	}
	return degrees;
}
void cc_AssistedHandling() {
	motion_AssistedHandling();
}

TRAJ_STATE launchAndWait(RobotCommand* cmd) {
	checkRobotCommand(cmd);
	path_LaunchTrajectory(cmd);

	TRAJ_STATE result = path_WaitEndOfTrajectory();
	printf("path_WaitEndOfTrajectory returned : %d : (TRAJ_OK=%d TRAJ_COLLISION=%d)\n", result, TRAJ_OK, TRAJ_COLLISION);
	free(cmd);
	return result;
}

// if distance <0, move backward
TRAJ_STATE cc_move(float distanceInMM, float VMax, float Accel, float Decel) {


//printf("\ncc_move old dist=%f  patch ferte 2015\n", distanceInMM);
	distanceInMM = distanceInMM + (20.0 * distanceInMM/1000.0) ;
//printf("\ncc_move new dist=%f patch ferte 2015\n", distanceInMM);


	if (distanceInMM > 0) {
		ignoreRearCollision = TRUE;
	} else {
		ignoreFrontCollision = TRUE;
	}
	RobotCommand* cmd = (RobotCommand*) malloc(sizeof(RobotCommand));
	float meters = distanceInMM / 1000.0f;
	motion_LineSpeedAcc(cmd, meters, VMax, Accel, Decel);
	return launchAndWait(cmd);
}

TRAJ_STATE cc_moveForwardTo(float xMM, float yMM, float VMax, float Accel, float Decel) {
	float dx = cc_getRelativeX(xMM) - cc_getX();
	float dy = yMM - cc_getY();
	float aRadian = atan2(dy, dx);
	TRAJ_STATE ts = cc_rotateTo(cc_getRelativeAngle((aRadian * 180.0f) / M_PI), cc_motion_GetDefaultSpeedForRotation(), cc_motion_GetDefaultAccelForRotation(), cc_motion_GetDefaultDecelForRotation());
	float dist = sqrt(dx * dx + dy * dy);
	return cc_move(dist, VMax, Accel, Decel);
}
TRAJ_STATE cc_moveForwardAndRotateTo(float xMM, float yMM, float thetaInDegree) {
	TRAJ_STATE ts;
	ts = cc_moveForwardTo(xMM, yMM, cc_motion_GetDefaultSpeed(), cc_motion_GetDefaultAccel(), cc_motion_GetDefaultDecel());
	if (ts != TRAJ_OK)
		return ts;
	//printf("thetaInDegree = %f\n", thetaInDegree);
	ts = cc_rotateTo(thetaInDegree, cc_motion_GetDefaultSpeedForRotation(), cc_motion_GetDefaultAccelForRotation(), cc_motion_GetDefaultDecelForRotation());
	return ts;
}
TRAJ_STATE cc_moveBackwardTo(float xMM, float yMM, float VMax, float Accel, float Decel) {
	if (cc_getMatchColor() != 0) {
		xMM = 3000 - xMM;
	}
	float dx = xMM - cc_getX();
	float dy = yMM - cc_getY(); //-500 -x = -900
	float aRadian = atan2(dy, dx);

	TRAJ_STATE ts = cc_rotateTo(cc_getRelativeAngle(((M_PI + aRadian) * 180.0f) / M_PI), cc_motion_GetDefaultSpeedForRotation(), cc_motion_GetDefaultAccelForRotation(), cc_motion_GetDefaultDecelForRotation()); //TODO angle au plus court ?

	float dist = sqrt(dx * dx + dy * dy);
	return cc_move(-dist, VMax, Accel, Decel);
}
TRAJ_STATE cc_moveBackwardAndRotateTo(float xMM, float yMM, float thetaInDegree) {
	TRAJ_STATE ts;
	ts = cc_moveBackwardTo(xMM, yMM, cc_motion_GetDefaultSpeed(), cc_motion_GetDefaultAccel(), cc_motion_GetDefaultDecel());
	if (ts != TRAJ_OK)
		return ts;
	ts = cc_rotateTo(thetaInDegree, cc_motion_GetDefaultSpeedForRotation(), cc_motion_GetDefaultAccelForRotation(), cc_motion_GetDefaultDecelForRotation());
	return ts;
}
TRAJ_STATE cc_rotateAbs(float degrees, float VMax, float Accel, float Decel) {
	int f = ignoreFrontCollision;
	int b = ignoreRearCollision;
	ignoreFrontCollision = TRUE;
	ignoreRearCollision = TRUE;

	//patch ferté 2015
	if(degrees>0)
		degrees = degrees + (10.0 * degrees/180.0) ;
	else
		degrees = degrees - (10.0 * degrees/180.0) ;

	//fin patch

	if (cc_getMatchColor() != 0) {
		degrees = -degrees;
	}
	//reduction de 0 à 360
	if (degrees > 360) {
		degrees = ((int) (degrees * 1000.0f) % 360000) / 1000.0f;
	}
	if (degrees < -360) {
		int d = (int) -(degrees * 1000.0f);
		d = d % 360000;
		degrees = -d / 1000.0f;
	}

	// OPTIMISER la rotation -PI < a < PI
	if (degrees >= 180)
		degrees -= 360;
	if (degrees < -180)
		degrees += 360;

	RobotCommand* cmd = (RobotCommand*) malloc(sizeof(RobotCommand));
	float rad = (degrees * M_PI) / 180.0f;
	motion_RotateSpeedAcc(cmd, rad, VMax, Accel, Decel);
	TRAJ_STATE ts = launchAndWait(cmd);
	ignoreFrontCollision = f;
	ignoreRearCollision = b;
	return ts;
}
TRAJ_STATE cc_rotateLeft(float degrees) {
	return cc_rotateAbs(degrees, cc_motion_GetDefaultSpeedForRotation(), cc_motion_GetDefaultAccelForRotation(), cc_motion_GetDefaultDecelForRotation());
}
TRAJ_STATE cc_rotateRight(float degrees) {
	return cc_rotateAbs(-degrees, cc_motion_GetDefaultSpeedForRotation(), cc_motion_GetDefaultAccelForRotation(), cc_motion_GetDefaultDecelForRotation());
}

TRAJ_STATE cc_rotateTo(float thetaInDegree, float VMax, float Accel, float Decel) {

	float currentThetaInDegree = cc_getThetaInDegree();

	float delta = cc_getRelativeAngle(thetaInDegree) - currentThetaInDegree;

	float turn = ((int) (delta * 1000.0f) % 360000) / 1000.0f;
	//printf("\nccbase.c cc_rotateTo %f deg   current=%f   delta=%f   turn=%f\n", thetaInDegree, currentThetaInDegree, delta, turn);


//printf("\ncc_rotateTo old degrees=%f  patch ferte 2015\n", turn);

	//if (turn>=0)
	//if(turn <=130)
		//turn = turn + (10.0 * turn/180.0) ;

	//else
		//turn = turn + (5.0 * turn/180.0) ;
//printf("cc_rotateTo new degrees=%f patch ferte 2015\n", turn);


	//printf("ccbase.c cc_rotateAbs %f deg   delta=%f deg\n ", turn, delta);
	if (cc_getMatchColor() != 0) {
		turn = -turn;
	}
	TRAJ_STATE ts = cc_rotateAbs(turn, VMax, Accel, Decel); //cho use Abs not left!!

	return ts;
}

void cc_setPosition(float xMM, float yMM, float thetaDegrees, int matchColor) {
	if (matchColor != 0) {
		//yMM = -yMM;
		xMM = 3000 - xMM;
		thetaDegrees = 180.0 - thetaDegrees;
	}

	odo_SetPosition(xMM / 1000.0, yMM / 1000.0, thetaDegrees * M_PI / 180.0);
}

// position x,x in mm
float cc_getX() {
	RobotPosition p = odo_GetPosition();
	return p.x * 1000.0f;
}
float cc_getY() {
	RobotPosition p = odo_GetPosition();
	return p.y * 1000.0f;
}
// angle in radian
float cc_getTheta() {
	RobotPosition p = odo_GetPosition();
	return p.theta;
}
// angle in degrees
float cc_getThetaInDegree() {
	RobotPosition p = odo_GetPosition();
	return (p.theta * 180.0f) / M_PI;
}

void cc_setIgnoreFrontCollision(boolean b) {
	ignoreFrontCollision = b;
}
void cc_setIgnoreRearCollision(boolean b) {
	ignoreRearCollision = b;
}
void cc_collisionOccured(int front) {
	printf("TTTTTTTTTTTTTTEST : front=%d ignoreFrontCollision=%d ignoreRearCollision=%d\n", front, ignoreFrontCollision,
			ignoreRearCollision);
	if (front && !ignoreFrontCollision) {
		printf("===>cc_collisionOccured on front ignoreFrontCollision=%d\n ", ignoreFrontCollision);
		path_CollisionOnTrajectory();
	} else if (!front && !ignoreRearCollision) {
		printf("===>cc_collisionOccured on rear ignoreFrontCollision=%d\n ", ignoreRearCollision);
		path_CollisionRearOnTrajectory();
	}
}

void cc_setMatchColor(int color) {
	matchColor = color;
}
int cc_getMatchColor() {
	return matchColor;
}

void cc_setMirrorCoordinates(boolean b) {

}

TRAJ_STATE cc_goToZone(const char *zoneName) {
	ZONE* z = ia_getZone(zoneName);
	TRAJ_STATE ts;
	printf("%s (line %d) : goToZone %s\n", __FUNCTION__, __LINE__, zoneName);

	if (z == NULL) {
		printf("ERROR: %s %d : unable to get zone %s\n", __FUNCTION__, __LINE__, zoneName);
		return TRAJ_ERROR;
	}

	ZONE *zCurrent = ia_getNearestZoneFrom(cc_getX(), cc_getY());
	if (zCurrent == NULL) {
		printf("ERROR: cc_goToZone ia_getNearestZoneFrom return NULL !!");
		return TRAJ_ERROR;
	}

	ZONE_PATH *path = ia_getZonePath(zCurrent, z);

	if (path != NULL) {
		printf("%s (line %d) : goToZone FROM %s TO %s using path (%f,%f)\n", __FUNCTION__, __LINE__, zCurrent->name,
				z->name, path->x, path->y);
		ts = cc_moveForwardTo(cc_getRelativeX(path->x), path->y, cc_motion_GetDefaultSpeed(), cc_motion_GetDefaultAccel(), cc_motion_GetDefaultDecel());
		if (ts != TRAJ_OK) {
			return ts;
		}
	} else {
		printf("%s (line %d) : goToZone FROM %s TO %s with NO path \n", __FUNCTION__, __LINE__, zCurrent->name,
				z->name);
	}
	ts = cc_moveForwardAndRotateTo(cc_getRelativeX(z->startX), z->startY, cc_getRelativeAngle(z->startAngle));
	return ts;

}
