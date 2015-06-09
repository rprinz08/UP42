#ifndef _TOOLS_H_
#define _TOOLS_H_

#define LOG_NORMAL					0
#define LOG_INFO					1
#define LOG_DEBUG					3
#define LOG_MEM						4
#define LOG_COMM					5

#define MAX_STRING					1024

extern char *emptyString;

extern int fileExists(const char *);
extern unsigned long getMs();
extern void freeTableElement(void *);
extern void freeAllTable();
extern char *cloneString(const char *);
extern char *createString(int);
extern char *formatString(const char *, ...);
extern void printInfo(int, FILE *, const char *, ...);
extern void printError(FILE *, const char *, ...);
extern char *getTempFile(char *);
extern void showDump(int, FILE *, unsigned char *, unsigned int);

#endif
