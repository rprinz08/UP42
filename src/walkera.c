#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "walkera.h"
#include "serial.h"
#include "xmodem.h"
#include "tools.h"
#include "up02c.h"

int connect(HANDLE portHandle, int timeoutMs, char showStatus) {
	char inb = 0;
	int t = (timeoutMs < POLL_TIME) ? POLL_TIME : timeoutMs;
	unsigned long tx = getMs() + t;
	
	while(inb != '>' && getMs() < tx) {
		serial_outByte(portHandle, 'D');
		inb = serial_inByte(portHandle, POLL_TIME);
		if(verbosity < LOG_COMM)
			printInfo(LOG_NORMAL, stdout, ".");
	}
	
	return(inb == '>');
}

void disconnect(HANDLE portHandle) {
	serial_outByte(portHandle, 'Q');
}

int getInfo(HANDLE portHandle, int timeoutMs, char *buffer, int buffer_size) {
	int st;
	char inb = '>';
	int t = (timeoutMs < POLL_TIME) ? POLL_TIME : timeoutMs;
	unsigned long tx = getMs() + t;
	
	while(inb == '>' && getMs() < tx) {
		serial_outByte(portHandle, 'I');
		inb = serial_inByte(portHandle, POLL_TIME);
	}
	
	if(getMs() > tx)
		return -1;
		
	Sleep(200);

	st = xmodemReceive(buffer, buffer_size);
	buffer[buffer_size] = '\0';
	return st;
}

int flash(HANDLE portHandle, int timeoutMs, const char *fileName) {
	int st;
    struct stat xstat;
    char *buffer;
	
	FILE *fd = fopen(fileName, "rb");
    if(fd) {
        fstat(fileno(fd), &xstat);
        buffer = createString(xstat.st_size);
        fread(buffer, xstat.st_size, 1, fd);
        fclose(fd);
    }

	serial_outByte(portHandle, 'F');
	Sleep(200);
	
	st = xmodemTransmit(buffer, xstat.st_size);
	return st;
}


