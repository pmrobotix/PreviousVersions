/*
 * test.c
 *
 *  Created on: Dec 1, 2013
 *      Author: maillard
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include  <sys/mman.h>
#include <sys/ioctl.h>
#include "lms2012.h"

// A = 0x01, B = 0x02, C = 0x04, D = 0x08

int test_motor_encoder() {
	int motorSpeed = 19;
	int motorLeft = 0x02;
	int motorRight = 0x04;

	MOTORDATA *pMotorData;

	char motor_command[4];
	int motor_file;
	int encoder_file;
	int i;
	//Open the device file asscoiated to the motor controlers
	if ((motor_file = open(PWM_DEVICE_NAME, O_WRONLY)) == -1) {
		printf("Failed to open device\n");
		return -1; //Failed to open device
	}
	//Open the device file asscoiated to the motor encoders
	if ((encoder_file = open(MOTOR_DEVICE_NAME, O_RDWR | O_SYNC)) == -1)
		return -1; //Failed to open device
	pMotorData = (MOTORDATA*) mmap(0, sizeof(MOTORDATA) * vmOUTPUTS,
	PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, encoder_file, 0);
	if (pMotorData == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}
	// All motor operations use the first command byte to indicate the type of operation
	// and the second one to indicate the motor(s) port(s)
	motor_command[0] = opOUTPUT_SPEED;
	motor_command[1] = motorLeft;
	motor_command[2] = motorSpeed;
	write(motor_file, motor_command, 3);
	motor_command[0] = opOUTPUT_SPEED;
	motor_command[1] = motorRight;
	motor_command[2] = motorSpeed;
	write(motor_file, motor_command, 3);
	// Start the motor
	motor_command[0] = opOUTPUT_START;
	motor_command[1] = motorLeft | motorRight;
	write(motor_file, motor_command, 2);
	// Read encoders while running the motor
	int startLeft = pMotorData[1].TachoSensor;
	int startRight = pMotorData[2].TachoSensor;

	for (i = 1; i < 6; i++) {
		printf("Spd/Cnt/Snr: L=%d/%d/%d R=%d/%d/%d\n", pMotorData[1].Speed,
				pMotorData[1].TachoCounts, pMotorData[1].TachoSensor,
				pMotorData[2].Speed, pMotorData[2].TachoCounts,
				pMotorData[2].TachoSensor);
		printf("Left:%d Right:%d\n", pMotorData[1].TachoSensor - startLeft,
				pMotorData[2].TachoSensor - startRight);
		sleep(1);
	}
	// Stop the motor
	motor_command[0] = opOUTPUT_STOP;
	write(motor_file, motor_command, 2);
	// Close device files

	printf("Left:%d Right:%d\n", pMotorData[1].TachoSensor - startLeft,
			pMotorData[2].TachoSensor - startRight);

	close(encoder_file);
	close(motor_file);
	return 0;
}

void a(char motorPort) {

	//DEFINE VARIABLES
	char motor_command[3];
	int motor_file;

	//INITIALIZE DEVICE
	//Open the device file asscoiated to the motor controlers
	if ((motor_file = open(PWM_DEVICE_NAME, O_WRONLY)) == -1)
		return; //Failed to open device

	//ACTUATE MOTORS
	// All motor operations use the first command byte to indicate the type of operation
	// and the second one to indicate the motor(s) port(s)
	motor_command[0] = opOUTPUT_SPEED;
	motor_command[1] = motorPort;
	// Set the motor desired speed 0 to 100
	motor_command[2] = 100;
	//printf("moteur a 100 \n");
	write(motor_file, motor_command, 3);
	// Start the motor
	motor_command[0] = opOUTPUT_START;
	write(motor_file, motor_command, 2);
	// Run the motor for a couple of seconds
	sleep(2);
	// Stop the motor
	motor_command[0] = opOUTPUT_STOP;
	write(motor_file, motor_command, 2);

	//CLOSE DEVICE
	close(motor_file);

}
//=====================================================================
// ReadSensorHTAngle(port, Angle, AccAngle, RPM)
// Reads the HiTechnic Angle Sensor and returns the current:
//   Angle              degrees (0-359)
//   Accumulated Angle  degrees (-2147483648 to 2147483647)
//   RPM                rotations per minute (-1000 to 1000)
//typedef int bool;
//typedef char byte;
//
//
// void ReadSensorHTAngle(int port, int *Angle, long *AccAngle, int *RPM)
//{
//	int count;
//	byte cmndbuf[] = {0x02, 0x42};  // I2C device, register address
//	byte respbuf[60];// Response Buffer
//	bool fSuccess;
//	count=8;// 8 bytes to read
//	fSuccess = I2CBytes(port, cmndbuf, count, respbuf);
//	if (fSuccess) {
//		*Angle = respbuf[0]*2 + respbuf[1];
//		*AccAngle = respbuf[2]*0x1000000 + respbuf[3]*0x10000+
//		respbuf[4]*0x100 + respbuf[5];
//		*RPM = respbuf[6]*0x100 + respbuf[7];
//	} else {
//		// No data from sensor
//		Angle = 0; AccAngle = 0; RPM = 0;
//	}
//}
//
//// ReserSensorHTAnalog(port, resetmode)
//// resetmode:
////   HTANGLE_MODE_CALIBRATE  Calibrate the zero position of angle.
////                           Zero position is saved in EEPROM on sensor.
////   HTANGLE_MODE_RESET      Reset the rotation count of accumulated.
////                           angle to zero.  Not saved in EEPORM.
//#define HTANGLE_MODE_CALIBRATE              0x43
//#define HTANGLE_MODE_RESET                  0x52
//void ResetSensorHTAngle(int port, int resetmode) {
//	int count;
//	char cmndbuf[] = { 0x02, 0x41, 0 }; // I2C device, register address
//	char respbuf[60];                   // buffer for inbound I2C response
//	cmndbuf[2] = resetmode;           // Set reset code
//	count = 0;                          // 0 bytes to read
//	I2CBytes(port, cmndbuf, count, respbuf);
//	if (resetmode == HTANGLE_MODE_CALIBRATE)
//		sleep(1);  // Time to allow burning EEPROM
//}

int calibrate() {
	int portCounter1 = 2;
	//DEFINE VARIABLES
	int iic_device_file;
	IIC *pIic;
	int i;

	//INITIALIZE DEVICE
	//Open the device file
	if ((iic_device_file = open(IIC_DEVICE_NAME, O_RDWR | O_SYNC)) == -1) {
		printf("Failed to open IIC device\n");
		return -1;
	}
	pIic = (IIC*) mmap(0, sizeof(IIC), PROT_READ | PROT_WRITE,
	MAP_FILE | MAP_SHARED, iic_device_file, 0);
	if (pIic == MAP_FAILED) {
		printf("Mapping IIC device failed\n");
		return -1;
	}
	printf("IIC Device is ready\n");

	IICDAT IicDat;
	//SPECIAL CONFIGURATION
	//Setup IIC to read 2 packets
	IicDat.Result = OK;
	IicDat.Port = portCounter1;
	IicDat.Repeat = 0;
	IicDat.Time = 0;

	IicDat.WrLng = 2;
	// Set the device I2C address
	IicDat.WrData[0] = 0x01;
	// Specify the register that will be read (0x42 = angle)
	IicDat.WrData[1] = 0x42;

	//
	IicDat.RdLng = 8;
	IicDat.RdData[0] = 0;
	IicDat.RdData[1] = 0;
	IicDat.RdData[2] = 0;
	IicDat.RdData[3] = 0;
	IicDat.RdData[4] = 0;
	IicDat.RdData[5] = 0;
	IicDat.RdData[6] = 0;
	IicDat.RdData[7] = 0;

	// Setup I2C comunication
	int r = ioctl(iic_device_file, IIC_SETUP, &IicDat);
	printf("Device is ready %d \n", r);

	long angleCounter = 0;

	DATA8 *respbuf = pIic->Raw[portCounter1][pIic->Actual[portCounter1]];

	for (i = 0; i < 10000; i++) {

		respbuf = pIic->Raw[portCounter1][pIic->Actual[portCounter1]];
		int d0 = respbuf[0];
		int d1 = respbuf[1];
		int d2 = respbuf[2];
		int d3 = respbuf[3];
		int d4 = respbuf[4];
		int d5 = respbuf[5];
		int d6 = respbuf[6];
		int d7 = respbuf[7];

		//printf("E:%d\n", d1 + d6);
		printf("EV3: %d %d %d %d %d %d %d %d    \n", d0, d1, d2, d3, d4, d5, d6,
				d7);
		int angle = d1;
		int nbTour = d2;
		if (angle < 0) {
			angle = (256 + angle);

		}
		//printf("E: d3 corrige %d\n", d3);
		angleCounter = nbTour * 256 + angle;

		//printf("Last:%d d:%d \n", lastAngle, delta);
		printf("EV3: nbTour:%d angle:%d  t:%ld\n", nbTour, angle, angleCounter);
		//	uc =
		//			(unsigned char) (pIic->Raw[portCounter1][pIic->Actual[portCounter1]][0]);
		//printf("Value: %d \n", uc);
		//printf("Angle:%d last: %d Total: %ld delta:%d\n", angle, lastAngle,
		//	angleCounter, delta);

		usleep(500 * 1000);
		//sleep(1);
	}

	//CLOSE DEVICE
	printf("Clossing device\n");
	close(iic_device_file);

	return 0;
}

void testUart() {
	int buttonPort = 0;

	//DEFINE VARIABLES
	int file;
	UART *pUart; //UART porte user level handler
	int i;

	//INITIALIZE DEVICE
	//Open the device uart device file
	if ((file = open(UART_DEVICE_NAME, O_RDWR | O_SYNC)) == -1) {
		printf("Failed to open device\n");
		return;
	}
	//Map kernel device to be used at the user space level
	pUart = (UART*) mmap(0, sizeof(UART), PROT_READ | PROT_WRITE,
	MAP_FILE | MAP_SHARED, file, 0);
	if (pUart == MAP_FAILED) {
		printf("Failed to map device\n");
		return;
	}

	//PROCESS SENSOR DATA
	printf("Device is ready\n");
	for (i = 0; i < 100; i++) {
		printf("UART Value: %d\n",
				(unsigned char) pUart->Raw[buttonPort][pUart->Actual[buttonPort]][0]);
		sleep(1);
	}

	//CLOSE DEVICE
	printf("Clossing device\n");
	close(file);

}
void testButton2() {
	// Bouton poussoir : ANALOG, Pin1, Pressed if value < 225
	int buttonPort = 0;
//DEFINE VARIABLES
	int file;
	ANALOG *pAnalog;
	int i;

	//INITIALIZE DEVICE
	//Open the device file
	if ((file = open(ANALOG_DEVICE_NAME, O_RDWR | O_SYNC)) == -1) {
		printf("Failed to open device\n");
		return;
	}
	pAnalog = (ANALOG*) mmap(0, sizeof(ANALOG), PROT_READ | PROT_WRITE,
	MAP_FILE | MAP_SHARED, file, 0);
	if (pAnalog == MAP_FAILED) {
		printf("Mapping device failed\n");
		return;
	}
	printf("Device ready for Analog\n");

	//PROCESS SENSOR DATA
	//Read samples
	unsigned char v;
	for (i = 0; i < 100; i++) {
		v =
				(unsigned char) pAnalog->Pin1[buttonPort][pAnalog->Actual[buttonPort]];
		printf("Analog Value =  %d \n", v);
		if (v < 225) {
			printf("Pressed\n");
		} else {
			printf("Not Pressed\n");
		}
		sleep(1);
	}

	//CLOSE DEVICE
	printf("Closing device\n");
	close(file);
}
