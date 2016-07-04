#ifndef _WALKERA_H_
#define _WALKERA_H_

#define POLL_TIME		100

extern int connectBoard(HANDLE, int, char);
extern void disconnectBoard(HANDLE);
extern int getBoardInfo(HANDLE, int, char *, int);
extern int flashBoard(HANDLE, int, const char *);

#endif