#ifndef _UP02C_H_
#define _UP02C_H_

#define EXIT_OK						0
#define EXIT_INPUT_FILE_ERROR		1
#define EXIT_OUTPUT_FILE_ERROR		2
#define EXIT_KEY_ERROR				3
#define EXIT_PORT_ERROR				4
#define EXIT_BAUD_ERROR				5
#define EXIT_UNKNOWN_ERROR			9

#define VERSION     				"0.2"

extern void system_error(char *name);
extern char *getTempFile(char *prefix);

extern HANDLE _openPort(const char *port, int baud);
extern int _inbyte(unsigned short timeout); // msec timeout
extern void _outbyte(int c);
extern void _closePort(HANDLE h);
extern int rxTest(void);

#endif
