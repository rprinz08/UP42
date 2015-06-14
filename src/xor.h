#ifndef _XOR_H_
#define _XOR_H_

#define BUFFER_SIZE  8192

extern char *parseKey(char *, int *);
extern void xorFile(FILE *, FILE *, const char *, int);
extern int xor(const char *, const char *, char *);

#endif
