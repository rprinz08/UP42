
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "up02c.h"

#ifdef _WIN32
    typedef unsigned char u_char;
#endif

#define BUFFER_SIZE  8192



void std_err(void) {
    perror("\nError");
    exit(1);
}



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

        parsedKey = malloc((l / 2) + 1);
        if(!parsedKey)
            std_err();

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
        parsedKey = malloc(xstat.st_size);
        if(!parsedKey)
            std_err();
            
        fread(parsedKey, xstat.st_size, 1, fd);
        fclose(fd);
        *keyLen = xstat.st_size;
        
        return(parsedKey);
    }

    // key is a string of characters
    parsedKey = malloc(l);
    if(!parsedKey)
        std_err();
        
    memcpy(parsedKey, key, l);
    *keyLen = l;
    
    return(parsedKey);
}



void xorFile(FILE *inz, FILE *outz, u_char *key, int keyLen) {
    int len;
    u_char *buff, *p, *l, *k, *kl;
            
    kl = key + keyLen;
    fprintf(stderr, " (hex dump follows):\n");
    show_dump(key, keyLen, stderr);

    buff = malloc(BUFFER_SIZE + 1);
    if(!buff)
        std_err();

    fprintf(stderr, "- read and xor file\n");
    k = key;
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
}



int xor(char *inf, char *outf, u_char *key) {
    FILE *inz, *outz;
    int keyLen;
    u_char *parsedKey;

    setbuf(stdout, NULL);

    fprintf(stderr, "- input file: ");
    if(!strcmp(inf, "-")) {
        fprintf(stderr, "stdin\n");
        inz = stdin;
    } 
    else {
        fprintf(stderr, "%s\n", inf);
        inz = fopen(inf, "rb");
    }
    if(!inz)
        std_err();

    fprintf(stderr, "- output file: ");
    if(!strcmp(outf, "-")) {
        fprintf(stderr, "stdout\n");
        outz = stdout;
    } 
    else {
        fprintf(stderr, "%s\n", outf);
        outz = fopen(outf, "wb");
    }
    if(!outz)
        std_err();

    //parsedKey = parse_key(key, &keyLen);
    parseKey(key, &keyLen);
    
    xorFile(inz, outz, parsedKey, keyLen);
        
    if(inz != stdin)
        fclose(inz);
        
    if(outz != stdout)
        fclose(outz);
        
    fprintf(stderr, "- finished\n");
    return(0);
}











