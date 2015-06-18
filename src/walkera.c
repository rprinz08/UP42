#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "serial.h"
#include "walkera.h"
#include "xmodem.h"
#include "tools.h"
#include "up02c.h"

int connectBoard(HANDLE portHandle, int timeoutMs, char showStatus) {
	char inb = 0;
	int t = (timeoutMs < POLL_TIME) ? POLL_TIME : timeoutMs;
	//int ph = POLL_TIME / 2;

	unsigned long now = getMs();	// current timestamp in ms
	unsigned long te = now + t;		// end timestamp
	unsigned long l = 0;			// last wait window start timestamp
	unsigned long w = 0;			// current window size in ms

	while(inb != '>' && now < te) {
		w = now - l;
		if(w >= POLL_TIME) {
			l = now;

			serial_outByte(portHandle, 'D');
			if(verbosity < LOG_COMM)
				printInfo(LOG_NORMAL, stdout, ".");
		}

		inb = serial_inByte(portHandle, POLL_TIME);
		if(inb < -1)
			return 0;
		//if(inb == -1)
		//	delay(ph);

		now = getMs();
	}

	return(inb == '>');
}

void disconnectBoard(HANDLE portHandle) {
	serial_outByte(portHandle, 'Q');
}

int getBoardInfo(HANDLE portHandle, int timeoutMs, char *buffer, int buffer_size) {
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

	delay(200);

	st = xmodemReceive((unsigned char *)buffer, buffer_size);
	buffer[buffer_size] = '\0';
	return st;
}

int flashBoard(HANDLE portHandle, int timeoutMs, const char *fileName) {
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
	delay(200);

	st = xmodemTransmit((unsigned char *)buffer, xstat.st_size);
	return st;
}


