#ifndef _UP02C_H_
#define _UP02C_H_

#define EXIT_OK						0
#define EXIT_INPUT_FILE_ERROR		1
#define EXIT_OUTPUT_FILE_ERROR		2
#define EXIT_KEY_ERROR				3
#define EXIT_PORT_ERROR				4
#define EXIT_BAUD_ERROR				5
#define EXIT_CONFIG_FILE_ERROR		6
#define EXIT_COMM_ERROR				7
#define EXIT_UNKNOWN_ERROR			9

#define LOG_NORMAL					0
#define LOG_INFO					1
#define LOG_DEBUG					3
#define LOG_MEM						4
#define LOG_COMM					5

#define VERSION     				"0.2"
#define MAX_STRING					1024


extern int verbosity;
extern int quiet;
extern char *emptyString;

extern int fileExists(char *s);
extern char *createString(int len);
extern char *formatString(const char *message, ...);
extern void printInfo(int level, FILE *file, const char *message, ...);
extern void printError(FILE *file, const char *message, ...);
extern char *getTempFile(char *prefix);

extern HANDLE _openPort(const char *port, int baud);
extern int _inbyte(unsigned short timeout); // msec timeout
extern void _outbyte(int c);
extern void _closePort(HANDLE h);
extern int rxTest(void);

#endif
