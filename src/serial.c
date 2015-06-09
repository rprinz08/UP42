
#include <stdio.h> 
#include <stdlib.h>

#ifdef _WIN32 
#include <windows.h>
#endif

#include "up02c.h"
#include "serial.h"
#include "tools.h"

HANDLE _portHandle = INVALID_HANDLE_VALUE;

HANDLE serial_openPort(const char *portName, int baud, 
		unsigned char dataBits, unsigned char parity) {
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
}

void serial_closePort(HANDLE portHandle) {
	CloseHandle(portHandle);
	_portHandle = INVALID_HANDLE_VALUE;
}

int _inbyte(unsigned short timeout) {
	return serial_inByte(_portHandle, timeout);
}

int serial_inByte(HANDLE portHandle, unsigned short timeout) {
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
}

void _outbyte(int c) {
	serial_outByte(_portHandle, (unsigned char)c);
}

void serial_outByte(HANDLE portHandle, unsigned char c) {
	unsigned char ch = c;
	DWORD written = 0;
	WriteFile(portHandle, &ch, 1, &written, NULL);
	printInfo(LOG_COMM, stdout,
		"> %c 0x%02x\n", (ch < 31 ? '.' : ch), ch);
}

int serial_setDTR(HANDLE portHandle, unsigned char DTR) {
	return (!EscapeCommFunction(portHandle, (DTR ? SETDTR : CLRDTR)));
}
