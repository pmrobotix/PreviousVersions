/*
 * SerialPort.h
 *
 *  Created on: Jun 20, 2011
 *      Author: jose
 */

#ifndef HOSTSERIALBUS_H_
#define HOSTSERIALBUS_H_

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

class HostSerialBus {
private:
	 int fileDescriptor;

   public:
	 int connect ();
	 int connect (const char * device);
	 void disconnect(void);

	 int sendArray(unsigned char *buffer, int len);
	 int getArray (unsigned char *buffer, int len);

	 int bytesToRead();
	 void clear();
};


#endif /* SERIALPORT_H_ */