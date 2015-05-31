#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

extern int verbosity;
extern int quiet;

void printInfo(FILE *file, const char *message, ...) {
	if(quiet)
		return;
		
	va_list argp;
	
	va_start(argp, message);
	vfprintf(file, message, argp);
	va_end(argp);
}

#if _WIN32
void system_error(char *name) {
	// Retrieve, format, and print out a message from the last error.  The 
	// `name' that's passed should be in the form of a present tense noun 
	// (phrase) such as "opening file".
	//
	char *ptr = NULL;
	//WCHAR ptr[1024];
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM,
	    0,
	    GetLastError(),
	    0,
	    (char *)&ptr,
	    //ptr,
	    1024,
	    NULL);
	
	//fprintf(stderr, "\nError %s: %s\n", name, ptr);
	//fprintf(stderr, "\nError %s: %s\n", name, &ptr);
	printInfo(stderr, "\nError %s: %s\n", name, ptr);
	LocalFree(ptr);
}

char *getTempFile(char *prefix) {
	DWORD dwRetVal = 0;
    UINT uRetVal = 0;
	int l;
	char lpTempPathBuffer[MAX_PATH];
	char szTempFileName[MAX_PATH];
	char *result; 
	
	dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer); 
    if(dwRetVal > MAX_PATH || (dwRetVal == 0))
        system_error("creating temp file");
	
	uRetVal = GetTempFileName(lpTempPathBuffer, prefix, 
                              0, szTempFileName); 
    if(uRetVal == 0)
        system_error("creating temp file");
	
	l = strlen(szTempFileName);
	result = (char *)malloc(l + 1);
	if(!result)
		system_error("creating temp file");
		
	memset(result, 0, l + 1);
	strncpy(result, szTempFileName, l);
	
	return result;
}
#endif
