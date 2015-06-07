
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "e4c.h"
#include "up02c.h"

#define BUFFER_SIZE  8192



char *parseKey(char *key, int *keyLen) {
    FILE *fd;
    struct stat xstat;
    int i, t, l;
    char *parsedKey, *k;

    l = strlen(key);

    // key is a hex string starting with '0x'
    if((key[0] == '0') && (tolower(key[1]) == 'x')) {
        for(i = 0; i < l; i++)
            key[i] = tolower(key[i]);

        parsedKey = createString((l / 2) + 1);
        k = parsedKey;
        key += 2;
        for(;;) {
            if(sscanf(key, "%02x", &t) != 1) 
                break;
                
            key += 2;
            *k++ = t;
            
            while(*key && (*key <= ' '))
                key++;
            
            if((*key == '0') && (key[1] == 'x')) 
                key += 2;
            if((*key >= '0') && (*key <= '9'))
                continue;
            if((*key >= 'a') && (*key <= 'f'))
                continue;
                
            break;
        }
        
        *keyLen = k - parsedKey;
        return(parsedKey);
    }

    // key is a file
    fd = fopen(key, "rb");
    if(fd) {
        fstat(fileno(fd), &xstat);
        parsedKey = createString(xstat.st_size);
        fread(parsedKey, xstat.st_size, 1, fd);
        fclose(fd);
        
        *keyLen = xstat.st_size;        
        return(parsedKey);
    }

    // key is a string of characters
    parsedKey = createString(l);
    memcpy(parsedKey, key, l);
    
    *keyLen = l;    
    return(parsedKey);
}



void xorFile(FILE *inz, FILE *outz, char *key, int keyLen) {
    int len;
    char *buff, *p, *l, *k, *kl;
            
    k = key;
    kl = key + keyLen;
    buff = createString(BUFFER_SIZE + 1);
    
    while((len = fread(buff, 1, BUFFER_SIZE, inz))) {
        for(p = buff, l = buff + len; p != l; p++, k++) {
            if(k == kl)
                k = key;
            *p ^= *k;
        }

        if(fwrite(buff, len, 1, outz) != 1) {
            fprintf(stderr, "\nError: write error, probably the disk space is finished\n");
            exit(1);
        }
    }
    
    freeTableElement(buff);
}



int xor(char *inputFileName, char *outputFileName, char *key) {
    FILE *inputFile, *outputFile;
    int keyLen;
    char *parsedKey;

    setbuf(stdout, NULL);

    if(!strcmp(inputFileName, "-"))
        inputFile = stdin;
    else
        inputFile = fopen(inputFileName, "rb");

    if(!inputFile) {
        printError(stderr, "opening input file (%s)", inputFileName);
        return(EXIT_INPUT_FILE_ERROR);
    } 

    if(!strcmp(outputFileName, "-"))
        outputFile = stdout;
    else 
        outputFile = fopen(outputFileName, "wb");

    if(!outputFile) {
        printError(stderr, "opening output file (%s)", outputFileName);
        return(EXIT_OUTPUT_FILE_ERROR);
    } 

    try {
        parseKey(key, &keyLen);    
        xorFile(inputFile, outputFile, parsedKey, keyLen);
    }
    catch(RuntimeException) {
        const e4c_exception *exception = e4c_get_exception();
        e4c_print_exception(exception);
        return(EXIT_UNKNOWN_ERROR);
    }
    finally {        
        if(inputFile != stdin)
            fclose(inputFile);
        
        if(outputFile != stdout)
            fclose(outputFile);
    }
        
    return(EXIT_OK);
}











