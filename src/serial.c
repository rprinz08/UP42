
#include <stdio.h> 
#include <stdlib.h> 
#include <windows.h>

#include "up02c.h"

HANDLE _portHandle = INVALID_HANDLE_VALUE;

HANDLE _openPort(const char *portName, int baud, unsigned char dataBits, unsigned char parity) {
	int i; 
	DCB dcb = { 0 }; 
	//char *portName = "\\\\.\\COM13"; 
	
	HANDLE h = CreateFile(portName, 
		GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING, 0, NULL); 
	if(h == INVALID_HANDLE_VALUE) 
		return INVALID_HANDLE_VALUE; 
	 
	dcb.DCBlength = sizeof(dcb); 
	dcb.BaudRate = baud; 
	dcb.fBinary = 1; 
	dcb.Parity = parity; 
	dcb.StopBits = ONESTOPBIT; 
	dcb.ByteSize = dataBits; 
	
	if(!SetCommState(h, &dcb)) 
		return INVALID_HANDLE_VALUE;
	//_setDTR(h, 1);		

	_portHandle = h;
	return h;
}

void _closePort(HANDLE h) {
	CloseHandle(h);
	_portHandle = INVALID_HANDLE_VALUE;
}

int _inbyte(unsigned short timeout) {
	unsigned char ch = 0; 
	DWORD read = 0; 	
	COMMTIMEOUTS timeouts;
	
    timeouts.ReadIntervalTimeout = timeout;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = timeout;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    if(!SetCommTimeouts(_portHandle, &timeouts))
        printError(stderr, "setting port time-outs");	
	
	ReadFile(_portHandle, &ch, 1, &read, NULL); 
	printInfo(LOG_COMM, stdout,
		"< %3d %c 0x%02x\n", read, (ch < 31 ? '.' : ch), ch);
	
	if(read <= 0)
		return -1;
	return ch; 
}

void _outbyte(int c) {
	char ch = (char)c;
	DWORD written = 0;
	WriteFile(_portHandle, &ch, 1, &written, NULL);
	printInfo(LOG_COMM, stdout,
		"> %c 0x%02x\n", (ch < 31 ? '.' : ch), ch);
}

int _setDTR(HANDLE _portHandle, char DTR) {
	return (!EscapeCommFunction(_portHandle, (DTR ? SETDTR : CLRDTR)));
}
