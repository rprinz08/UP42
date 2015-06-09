#ifndef _WALKERA_H_
#define _WALKERA_H_

#define POLL_TIME		500

extern int connect(HANDLE, int, char);
extern void disconnect(HANDLE);
extern int getInfo(HANDLE, int, char *, int);
extern int flash(HANDLE, int, const char *);

#endif