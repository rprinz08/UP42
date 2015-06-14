
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef linux
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#endif

#include "up02c.h"
#include "serial.h"
#include "tools.h"

HANDLE _portHandle = INVALID_HANDLE_VALUE;
#ifdef linux
struct termios oldtio, newtio;
#endif

HANDLE serial_openPort(const char *portName, int baud,
		unsigned char parity, unsigned char dataBits, unsigned char stopBits) {
#ifdef _WIN32
	int i; 
	DCB dcb = { 0 }; 
	//char *portName = "\\\\.\\COM13"; 

	HANDLE portHandle = CreateFile(portName, 
		GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING, 0, NULL); 
	if(portHandle == INVALID_HANDLE_VALUE) 
		return INVALID_HANDLE_VALUE; 

	dcb.DCBlength = sizeof(dcb); 
	dcb.BaudRate = baud; 
	dcb.fBinary = 1; 
	dcb.Parity = parity; 
	dcb.StopBits = ONESTOPBIT; 
	dcb.ByteSize = dataBits; 

	if(!SetCommState(portHandle, &dcb)) 
		return INVALID_HANDLE_VALUE;

	_portHandle = portHandle;
	return portHandle;
#endif
#ifdef linux
	long BAUD;
	long DATABITS;
	long STOPBITS;
	long PARITYON;
	long PARITY;

	switch(baud) {
		case 38400:
		default:
			BAUD = B38400;
			break;
		case 19200:
			BAUD  = B19200;
			break;
		case 9600:
			BAUD  = B9600;
			break;
		case 4800:
			BAUD  = B4800;
			break;
		case 2400:
			BAUD  = B2400;
			break;
		case 1800:
			BAUD  = B1800;
			break;
		case 1200:
			BAUD  = B1200;
			break;
		case 600:
			BAUD  = B600;
			break;
		case 300:
			BAUD  = B300;
			break;
		case 200:
			BAUD  = B200;
			break;
		case 150:
			BAUD  = B150;
			break;
		case 134:
			BAUD  = B134;
			break;
		case 110:
			BAUD  = B110;
			break;
		case 75:
			BAUD  = B75;
			break;
		case 50:
			BAUD  = B50;
			break;
	}

	switch(dataBits) {
		case 8:
		default:
			DATABITS = CS8;
			break;
		case 7:
			DATABITS = CS7;
			break;
		case 6:
			DATABITS = CS6;
			break;
		case 5:
			DATABITS = CS5;
			break;
	}

	switch(stopBits) {
		case 1:
		default:
			STOPBITS = 0;
			break;
		case 2:
			STOPBITS = CSTOPB;
			break;
	}

	switch(parity)	{
		case 0:
		default:
			PARITYON = 0;
			PARITY = 0;
			break;
		case 1:
			PARITYON = PARENB;
			PARITY = PARODD;
			break;
		case 2:
			PARITYON = PARENB;
			PARITY = 0;
			break;
	}

	_portHandle = open(portName, O_RDWR | O_NOCTTY);
	if(_portHandle < 0)
		return INVALID_HANDLE_VALUE;

	// save current port settings
	tcgetattr(_portHandle, &oldtio);

	// set new port settings for canonical input processing 
	newtio.c_cflag = BAUD | CRTSCTS | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;       //ICANON;
	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;
	tcflush(_portHandle, TCIFLUSH);
	tcsetattr(_portHandle, TCSANOW, &newtio);

	return _portHandle;
#endif
}

void serial_closePort(HANDLE portHandle) {
#ifdef _WIN32
	CloseHandle(portHandle);
#endif
#ifdef linux
	tcsetattr(portHandle, TCSANOW, &oldtio);
	close(portHandle);
#endif
	_portHandle = INVALID_HANDLE_VALUE;
}

int _inbyte(unsigned short timeout) {
	return serial_inByte(_portHandle, timeout);
}

int serial_inByte(HANDLE portHandle, unsigned short timeout) {
#ifdef _WIN32
	unsigned char ch = 0; 
	DWORD read = 0; 	
	COMMTIMEOUTS timeouts;
	
    timeouts.ReadIntervalTimeout = timeout;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = timeout;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    if(!SetCommTimeouts(portHandle, &timeouts))
        printError(stderr, "setting port time-outs");	
	
	ReadFile(portHandle, &ch, 1, &read, NULL); 
	printInfo(LOG_COMM, stdout,
		"< %3d %c 0x%02x\n", read, (ch < 31 ? '.' : ch), ch);
	
	if(read <= 0)
		return -1;
	return ch;
#endif
#ifdef linux
	delay(500);
    return 0;
#endif
}

void _outbyte(int c) {
	serial_outByte(_portHandle, (unsigned char)c);
}

void serial_outByte(HANDLE portHandle, unsigned char c) {
#ifdef _WIN32
	unsigned char ch = c;
	DWORD written = 0;
	WriteFile(portHandle, &ch, 1, &written, NULL);
	printInfo(LOG_COMM, stdout,
		"> %c 0x%02x\n", (ch < 31 ? '.' : ch), ch);
#endif
#ifdef linux
#endif
}

int serial_setDTR(HANDLE portHandle, unsigned char DTR) {
#ifdef _WIN32
	return (!EscapeCommFunction(portHandle, (DTR ? SETDTR : CLRDTR)));
#endif
#ifdef linux
	return 0;
#endif
}
