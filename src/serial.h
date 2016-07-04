#ifndef _SERIAL_H_
#define _SERIAL_H_

#ifndef _WIN32
#define NOPARITY 					0
#define ODDPARITY 					1
#define EVENPARITY 					2
#define MARKPARITY 					3
#define SPACEPARITY 				4

#define ONESTOPBIT 					0
#define ONE5STOPBITS 				1
#define TWOSTOPBITS 				2

#define HANDLE 						int
#define INVALID_HANDLE_VALUE		0
#endif

extern HANDLE serial_openPort(const char *, int, unsigned char, unsigned char, unsigned char); 
#ifdef linux
extern void serial_resetPort(HANDLE);
#endif
extern void serial_closePort(HANDLE);
extern int _inbyte(unsigned short);
extern int serial_inByte(HANDLE, unsigned short);
extern void _outbyte(int);
extern void serial_outByte(HANDLE, unsigned char);
extern int serial_setDTR(HANDLE, unsigned char);

#endif
