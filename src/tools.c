#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

extern int verbosity;
extern int quiet;
char lastMessage[1024];

/*------------------------------------------------------------
        checks if a file exists and is readable
  ------------------------------------------------------------*/
int fileExists(char *s)
{
	FILE *f;

	if( (f=fopen(s,"r")) != NULL ) {
		fclose(f);
		return(1);
	}
	return(0);
}

char *stringFormat(const char *message, ...) {
	va_list argp;
	
	memset(lastMessage, 0, 1024);
	va_start(argp, message);
	vsnprintf(lastMessage, 1024, message, argp);
	lastMessage[1023] = '\0';
	va_end(argp);
	
	return lastMessage;
}

void printInfo(FILE *file, const char *message, ...) {
	if(quiet)
		return;
		
	va_list argp;
	
	va_start(argp, message);
	vfprintf(file, message, argp);
	va_end(argp);
}

#if _WIN32
void printError(FILE *file, const char *message, ...) {
	if(quiet)
		return;
		
	va_list argp;
	
	va_start(argp, message);
	
	// Retrieve, format, and print out a message from the last error. The 
	// `message' that's passed should be in the form of a present tense noun 
	// (phrase) such as "opening file".
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/ms679351%28v=vs.85%29.aspx
	char *ptr = NULL;
	//WCHAR ptr[1024];
	char *buffer = NULL;
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
	
	buffer = LocalAlloc(LPTR, 1024);
	
	sprintf(buffer, "\nError %s: %s\n", message, ptr);
	vfprintf(stderr, buffer, argp);	

	LocalFree(ptr);
	LocalFree(buffer);
	va_end(argp);	
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
        printError(stderr, "creating temp file");
	
	uRetVal = GetTempFileName(lpTempPathBuffer, prefix, 
                              0, szTempFileName); 
    if(uRetVal == 0)
        printError(stderr, "creating temp file");
	
	l = strlen(szTempFileName);
	result = (char *)malloc(l + 1);
	if(!result)
		printError(stderr, "creating temp file");
		
	memset(result, 0, l + 1);
	strncpy(result, szTempFileName, l);
	
	return result;
}
#endif
