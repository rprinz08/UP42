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

#define VERSION					"0.2"
#ifdef _WIN32
#define OS						"Windows"
#else
#ifdef linux
#define OS						"Linux"
#else
#define OS						"unknown"
#endif
#endif

extern int verbosity;
extern int quiet;

#endif
