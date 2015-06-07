#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "e4c.h"
#include "up02c.h"

struct s_freeTableEntry {
	char *element;
	struct s_freeTableEntry *prev;
	struct s_freeTableEntry *next;
};
typedef struct s_freeTableEntry freeTableEntry;
freeTableEntry *freeTable = NULL;

/*------------------------------------------------------------
        checks if a file exists and is readable
  ------------------------------------------------------------*/
int fileExists(char *s) {
	FILE *f;

	if((f=fopen(s,"r")) != NULL) {
		fclose(f);
		return(1);
	}
	return(0);
}

unsigned long getMs() {
	FILETIME ft;
	LONGLONG ms;
	unsigned long l;
	GetSystemTimeAsFileTime(&ft);
	ms = ((LONGLONG)ft.dwLowDateTime + ((LONGLONG)(ft.dwHighDateTime) << 32LL)) / 10000;
	l = ms & 0x00000000FFFFFFFF;
//printf("%ul\n", l);
	return l;			
}

void addFreeTable(void *element) {
	if(element == NULL)
		return;
	freeTableEntry *e = (freeTableEntry *)calloc(1, sizeof(freeTableEntry));
	if(!e)
		throw(NotEnoughMemoryException, "Unable to create free table entry");
	e->element = element;
	e->next = NULL;
	e->prev = NULL;
	if(freeTable == NULL)
		freeTable = e;
	else {
		freeTable->next = e;
		e->prev = freeTable;
		freeTable = e;
	}
	
	printInfo(LOG_MEM, stdout, 
		"FreeTable: Added (%s)\n", element);
}

void freeTableElement(void *element) {
	if(element == NULL)
		return;

	freeTableEntry *e = freeTable;
	while(e != NULL && e != element)
		e = e->prev;
	if(e == element) {
		printInfo(LOG_MEM, stdout, 
			"FreeTable: Deleted (%s)\n", e->element);
		if(e->element != NULL)
			free(e->element);
		if(e->prev != NULL)
			e->prev->next = e->next;
		if(e->next != NULL)
			e->next->prev = e->prev;
		free(e);
	}
}

void freeAllTable() {
	freeTableEntry *e = freeTable;
	freeTableEntry *p;
	
	while(e != NULL) {
		p = e->prev;

		printInfo(LOG_MEM, stdout, 
			"FreeTable: Deleted (%s)\n", e->element);
		if(e->element != NULL)
			free(e->element);
		if(e->prev != NULL)
			e->prev->next = e->next;
		if(e->next != NULL)
			e->next->prev = e->prev;
		free(e);
		
		e = p;
	}
}

char *cloneString(char *s) {
	int l = 0;
	char *clone = NULL;
	
	if(s == NULL)
		return NULL;
	if(*s == '\0')
		return emptyString;
		
	l = strlen(s);
	clone = (char *)calloc(l + 1, 1);
	if(!clone)
		throw(NotEnoughMemoryException, "Unable to clone string");
	strncpy(clone, s, l);
	clone[l] = '\0';
	addFreeTable(clone);
	
	return clone;
}

char *createString(int len) {
	if(len <= 0)
		return emptyString;
		
	char *string = (char *)calloc(len, 1);
	if(!string)
		throw(NotEnoughMemoryException, "Unable to create string");
	addFreeTable(string);
	
	return string;
}

char *formatString(const char *message, ...) {
	char w[MAX_STRING];
	va_list argp;
	
	memset(w, 0, MAX_STRING);
	va_start(argp, message);
	vsnprintf(w, MAX_STRING, message, argp);
	w[MAX_STRING - 1] = '\0';
	va_end(argp);
	
	return cloneString(w);
}

void printInfo(int level, FILE *file, const char *message, ...) {
	if(verbosity < level || quiet)
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
	//WCHAR ptr[MAX_STRING];
	char *buffer = NULL;
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM,
	    0,	
	    GetLastError(),
	    0,
	    (char *)&ptr,
	    //ptr,
	    MAX_STRING,
	    NULL);
	
	buffer = LocalAlloc(LPTR, MAX_STRING);
	
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
	char *tempFileName; 
	
	dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer); 
    if(dwRetVal > MAX_PATH || (dwRetVal == 0))
        throw(NotEnoughMemoryException, "Unable creating temp file");
	
	uRetVal = GetTempFileName(lpTempPathBuffer, prefix, 
                              0, szTempFileName); 
    if(uRetVal == 0)
        throw(NotEnoughMemoryException, "Unable creating temp file");
	
	l = strlen(szTempFileName);
	tempFileName = (char *)calloc(l + 1, 1);
	if(!tempFileName)
		throw(NotEnoughMemoryException, "Unable creating temp file");
		
	strncpy(tempFileName, szTempFileName, l);
	addFreeTable(tempFileName);	
	return tempFileName;
}
#endif
