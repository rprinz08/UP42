
#include <stdio.h> 
#include <stdlib.h> 
#include <windows.h>
#include "up02c.h"

HANDLE _portHandle = INVALID_HANDLE_VALUE;

HANDLE _openPort(const char *portName, int baud) {
	int i; 
	DCB dcb = { 0 }; 
	//char *portName = "\\\\.\\COM13"; 
	//char *portName = "COM4";
	
	HANDLE h = CreateFile(portName, 
		GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING, 0, NULL); 
	if(h == INVALID_HANDLE_VALUE) { 
		printf("Failed to open port %s\n", portName); 
		return INVALID_HANDLE_VALUE; 
	} 
	printf("Opened port %s\n", portName);
	 
	dcb.DCBlength = sizeof(dcb); 
	dcb.BaudRate = baud; 
	dcb.fBinary = 1; 
	dcb.Parity = NOPARITY; 
	dcb.StopBits = ONESTOPBIT; 
	dcb.ByteSize = 8; 
	
	if(!SetCommState(h, &dcb)) { 
		printf("SetCommState failed\n"); 
		return INVALID_HANDLE_VALUE;
	}	
	printf("Port set to 9600,N,8,1\n");
	
	return h;
}

void _closePort(HANDLE h) {
	CloseHandle(h);
	printf("Port closed\n"); 
}

int _inbyte(unsigned short timeout) { // msec timeout
	unsigned char ch = 0; 
	DWORD read = 0; 	
	COMMTIMEOUTS timeouts;
	
    timeouts.ReadIntervalTimeout = timeout;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = timeout;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    if(!SetCommTimeouts(_portHandle, &timeouts))
        system_error("setting port time-outs");	
	
	ReadFile(_portHandle, &ch, 1, &read, NULL); 
	
	if(read <= 0)
		return -1;
	return ch; 
}

void _outbyte(int c) {
	char ch = (char)c;
	DWORD written = 0;
	WriteFile(_portHandle, &ch, 1, &written, NULL);
}

void _setDTR(HANDLE _portHandle, char DTR) {
	if(!EscapeCommFunction(_portHandle, (DTR ? SETDTR : CLRDTR)))
		system_error("changing DTR");
}

int rxTest(void) {
	HANDLE h = _openPort("COM4", 9600); 
	if(h == INVALID_HANDLE_VALUE)
		return 1;
		
	_portHandle = h; 

	for(int i = 0; i < 16; i++) { 
		int ch = _inbyte(1100);
		_outbyte(48 + i); 
		if(ch >= 0) { 
			printf("  0x%02x %c", ch, ch); 
			if(i % 8 == 7) { 
				printf("\n"); 
			} 
		}
		else
			printf("  Err ! "); 
	}
	
	_closePort(h);
	_portHandle = INVALID_HANDLE_VALUE;
	return 0; 
}
