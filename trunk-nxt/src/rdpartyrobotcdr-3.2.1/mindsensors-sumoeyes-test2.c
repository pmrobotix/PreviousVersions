#pragma config(Sensor, S2,     HTMSSUMO,            sensorLightActive)
#pragma config(Motor,  motorA,          RIGHT,         tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorB,          LEFT,          tmotorNormal, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*
 * $Id: mindsensors-sumoeyes-test2.c 127 2012-12-05 19:32:39Z xander $
 */

/**
 * mindsensors-sumoeyes.h provides an API for the Mindsensors Sumo Eyes Sensor.  This program
 * demonstrates how to use that API.
 *
 * Changelog:
 * - 0.1: Initial release
 *
 * Credits:
 * - Big thanks to Mindsensors for providing me with the hardware necessary to write and test this.
 *
 * License: You may use this code as you wish, provided you give credit where it's due.
 *
 * THIS CODE WILL ONLY WORK WITH ROBOTC VERSION 3.55 beta 2 AND HIGHER.
 * Xander Soldaat (xander_at_botbench.com)
 * 20 February 2011
 * version 0.4
 */

#include "drivers/mindsensors-sumoeyes.h"

 /*
  =============================================================================
  main task with some testing code

 */
task main() {
  // Standard range is set to short range
  bool shortrange = true;
  tObstacleZone zone = 0;
  nNxtButtonTask  = -2;

  eraseDisplay();

  nxtDisplayCenteredTextLine(0, "Mindsensors");
  nxtDisplayCenteredBigTextLine(1, "SumoEyes");
  nxtDisplayCenteredTextLine(3, "Test 2");
  nxtDisplayCenteredTextLine(5, "Press enter to");
  nxtDisplayCenteredTextLine(6, "switch between");
  nxtDisplayCenteredTextLine(7, "ranges");
  wait1Msec(2000);
  eraseDisplay();

  // Set the sensor to short range
  MSSUMOsetShortRange(HTMSSUMO);
  nxtDisplayRICFile(0, 0, "sumonone.ric");


  while(true) {
    if (time1[T1] > 1000) {
      if (shortrange == false) {
        // set the sensor to short range and display this
        MSSUMOsetShortRange(HTMSSUMO);
        nxtDisplayCenteredTextLine(7, "Short range");
        shortrange = true;
      } else {
        // set the sensor to long range and display this
        MSSUMOsetLongRange(HTMSSUMO);
        nxtDisplayCenteredTextLine(7, "Long range");
        shortrange = false;
      }
	    PlaySound(soundBeepBeep);
	    while(bSoundActive)
      time1[T1] = 0;
    }

    while(nNxtButtonPressed != kEnterButton) {
	    zone = MSSUMOreadZone(HTMSSUMO);

	    if (shortrange) {
		    switch (zone) {
		      case MSSUMO_NONE:  nxtDisplayRICFile(0, 0, "sumonone.ric");  break;
		      case MSSUMO_FRONT: nxtDisplayRICFile(0, 0, "sumoSF.ric");  break;
		      case MSSUMO_RIGHT:  nxtDisplayRICFile(0, 0, "sumoSL.ric");  break;
		      case MSSUMO_LEFT: nxtDisplayRICFile(0, 0, "sumoSR.ric");  break;
		    }
		    nxtDisplayCenteredTextLine(7, "Short range");
	    } else {
		    switch (zone) {
		      case MSSUMO_NONE:  nxtDisplayRICFile(0, 0, "sumonone.ric");  break;
		      case MSSUMO_FRONT: nxtDisplayRICFile(0, 0, "sumoLF.ric");  break;
		      case MSSUMO_RIGHT:  nxtDisplayRICFile(0, 0, "sumoLL.ric");  break;
		      case MSSUMO_LEFT: nxtDisplayRICFile(0, 0, "sumoLR.ric");  break;
		    }
		    nxtDisplayCenteredTextLine(7, "Long range");
	    }
	    wait1Msec(10);
	  }
  }
}

/*
 * $Id: mindsensors-sumoeyes-test2.c 127 2012-12-05 19:32:39Z xander $
 */
