#ifndef _IHEX_H_
#define _IHEX_H_

// max intel hex memory
#define MAX_IHEX_MEM 65536

// the maximum number of bytes to put in one line
#define MAXHEXLINE 16

extern unsigned char memory[MAX_IHEX_MEM];

// this is used by load_file to get each line of intex hex
int parseHexLine(char *theline, int bytes[], int *addr, 
		int *num, int *code);

// this loads an intel hex file into the memory[] array
int loadHexFile(const char *filename);

// this writes a part of memory[] to an intel hex file
void saveHexFile(char *command);

// this does the dirty work of writing an intel hex file
// caution, static buffering is used, so it is necessary
// to call it with end=1 when finsihed to flush the buffer
// and close the file
void hexOut(FILE *fhex, int byte, int memory_location, int end);

// converts hex to binary file
const char *hex2bin(const char *hexFileName, const char *binFileName, int *byteLength);

#endif
