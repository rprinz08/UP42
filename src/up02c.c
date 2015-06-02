#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif

//#define E4C_THREADSAFE
#include "e4c.h"

#include "gopt.h"
#include "show_dump.h"
#include "up02c.h"

int verbosity = 0;
int quiet = 0;
char freeTempFile = 0;
char freeConfigFile = 0;


void exitProgram(int exitCode) {
    e4c_context_end();
    exit(exitCode);
}

 
 
void UncaughtExceptionHandler(const e4c_exception *exception) {
    printInfo(stderr, "\nError %s (%s)\n", exception->message, exception->name);
    exitProgram(EXIT_UNKNOWN_ERROR);
}



void showVersion() {
    printf("Walkera UP02 compatible firmware flasher.\n");
    printf("Version %s, Copyright 2015, richard.prinz@min.at\n", VERSION);    
}



void showUsage(char *prgName, int exitCode) {
    printf("\n");
    showVersion();
    printf("\n");
    printf("Usage: %s [-i inputFile] [-o outputFile] [-k Key] [-p Port] [-b Baud]\n\n",
                prgName);
    printf("-h,--help      Show this help screen.\n");
    printf("-V,--version   Show version information.\n");
    printf("-v,--verbose   Enable verbose mode.\n");
    printf("-c,--config    Config file. Defaults to %s.ini\n", prgName);
    printf("-m,--model     Use model profile from config file\n");
    printf("-q,--quiet     Be quiet. Dont output anything.\n");
    printf("-i,--input     Input file to send. If omitted or '-' stdin will \n");
    printf("               be used\n");
    printf("-o,--output    Output file after encryption with key. If '-' \n");
    printf("               stdout will be used.\n");
    printf("               If no key specified outputFile == inputFile.\n");
    printf("-k,--key       Key to XOR-encrypt inputFile with. Can be either:\n");
    printf("               * a hex string starting with '0x'\n");
    printf("               * the name of a file which content is used as key\n");
    printf("               * if not hex or file the character string itself\n");
    printf("               If omitted no encryption is performed on input\n");
    printf("-p,--port      Name of the port to send the encrypted output file\n");
    printf("               via XModem to. If omitted nothing will be sent\n");
    printf("-b,--baud      Baude rate to use. Defaults to 9600.\n");
    printf("\n");
    
    if(exitCode >= 0)
        exitProgram(exitCode);
}



int main(int argc, const char **argv) {
    e4c_context_begin(E4C_FALSE);
    e4c_context_set_handlers(UncaughtExceptionHandler, NULL, NULL, NULL);
    
    const char *prgName = (char *)*argv;
    char *w = NULL;
    char wb[1024];
    int l = 0;
    
    const char *argument = NULL;
    
    const char *configFileName = NULL;
    const char *modelName = NULL;
    
    const char *inputFileName = NULL;
    FILE *inputFile;
    
    const char *outputFileName = NULL;
    FILE *outputFile;
    
    const char *transferFileName = NULL;
    FILE *transferFile;
    
    const char *key = NULL;
    char *parsedKey;
    int keyLen;
    
    const char *port = NULL;
    
    int baud = 0;
    
    setbuf(stdout, NULL);
    
    
    // configure command-line options parsing
    void *options = gopt_sort(&argc, argv, gopt_start(
        gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help")),
        gopt_option('V', 0, gopt_shorts('V'), gopt_longs("version")),
        gopt_option('q', 0, gopt_shorts('q'), gopt_longs("quiet")),
        gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose")),
        gopt_option('c', GOPT_ARG, gopt_shorts('c'), gopt_longs("config")),
        gopt_option('m', GOPT_ARG, gopt_shorts('m'), gopt_longs("model")),
        gopt_option('i', GOPT_ARG, gopt_shorts('i'), gopt_longs("input")),
        gopt_option('o', GOPT_ARG, gopt_shorts('o'), gopt_longs("output")),
        gopt_option('k', GOPT_ARG, gopt_shorts('k'), gopt_longs("key")),
        gopt_option('p', GOPT_ARG, gopt_shorts('p'), gopt_longs("port")),
        gopt_option('b', GOPT_ARG, gopt_shorts('b'), gopt_longs("baud"))
    ));

    // get command line options
    if (gopt(options, 'h'))
        showUsage((char *)prgName, EXIT_OK);

    // show version
    if (gopt(options, 'V')) {
        showVersion();
        exitProgram(EXIT_OK);
    }
    
    // get verbosity level
    verbosity = gopt(options, 'v');

    // check if we should be quiet
    quiet = gopt(options, 'q');

    // get config file name    
    gopt_arg(options, 'c', &configFileName);
    if(!configFileName) {
        l = strlen(prgName);
        configFileName = malloc(l + 5);
        if(!configFileName) {
            printError(stderr, "opening config file (%s)", configFileName);
            exitProgram(EXIT_CONFIG_FILE_ERROR);
        }
        freeConfigFile = 1;
        memset((char *)configFileName, 0, l + 5);
        strncpy((char *)configFileName, prgName, l);
        strncat((char *)configFileName, ".ini", 4);
    }
    printInfo(stdout, "Config file (%s)\n", configFileName);
    if(!fileExists((char *)configFileName)) {
        printInfo(stdout, "Config file (%s) does not exist or is not readable - ignored\n", 
            configFileName);
        if(freeConfigFile)
            free((char *)configFileName);
        configFileName = NULL;
    }

//throw(RuntimeException, stringFormat("opening config file (%s)", "AAAA"));
    
    // get model parameters
    if(gopt_arg(options, 'm', &modelName)) {
        printInfo(stdout, "Model (%s)\n", modelName);
        if(get_private_profile_string(modelName, "key", NULL,
                               &wb, 1024, configFileName)) {
            printf("model key (%s)", wb);
        }        
    }
    
    // get serial port
    gopt_arg(options, 'p', &port);

    // get input file name    
    gopt_arg(options, 'i', &inputFileName);
    if(inputFileName) {
        if(!strcmp(inputFileName, "-"))
            inputFile = stdin;
        else
            inputFile = fopen(inputFileName, "rb");
            
        if(!inputFile) {
            printError(stderr, "opening input file (%s)", inputFileName);
            exitProgram(EXIT_INPUT_FILE_ERROR);
        }
    }
    else {
        printInfo(stderr, "\nError: No input file specified!\n");
        exitProgram(EXIT_INPUT_FILE_ERROR);
    }
    printInfo(stdout, "Input file (%s)\n", inputFileName);

    // get output file name
    gopt_arg(options, 'o', &outputFileName);
    if(!outputFileName) {
        // create temp file as output file
        outputFileName = getTempFile("");
        freeTempFile = 1;
    }
    
    if(outputFileName) {
        if(!strcmp(outputFileName, "-")) {
            if(port) {
                printInfo(stderr, "\nError: stdout as output file can only be specified without port.\n");
                exitProgram(EXIT_OUTPUT_FILE_ERROR);
            }
            outputFile = stdout;
        }
        else
            outputFile = fopen(outputFileName, "wb");
            
        if(!outputFile) {
            printError(stderr, "opening output file (%s)", outputFileName);
            exitProgram(EXIT_OUTPUT_FILE_ERROR);
        }
    }
    printInfo(stdout, "Output file (%s)\n", outputFileName);

    // get key
    gopt_arg(options, 'k', &key);
    if(key) {
        parsedKey = (char *)parseKey(key, &keyLen);
        printInfo(stdout, "Encrypt input file with key (hex dump follows):\n");
        show_dump(parsedKey, keyLen, stderr);        
        printInfo(stdout, "Key: %s\n", key);
    }
    else {
        // create dummy key which does nothing
        parsedKey = "\0";
        keyLen = 1;
    }
    
    if(gopt_arg(options, 'b', &argument)) {
        baud = (int)strtol(argument, &w, 0);
        if(w == argument) {
            printInfo(stderr, "\nError: (%s) invalid baud rate\n", argument);
            exitProgram(EXIT_BAUD_ERROR);
        }
    }
    else
        baud = 9600;

    // done with processing command line arguments
    gopt_free(options);





    xorFile(inputFile, outputFile, parsedKey, keyLen);
    
    if(inputFile != stdin)
        fclose(inputFile);
        
    if(outputFile != stdout)
        fclose(outputFile);
            
    if(port) {
        int st;
        
        HANDLE portHandle = _openPort(port, baud);
        
        /* the following should be changed for your environment:
        0x30000 is the download address,
        12000 is the maximum size to be send from this address
        */
        st = xmodemTransmit((char *)0x30000, 12000);
        if (st < 0) {
            printf ("Xmodem transmit error: status: %d\n", st);
        }
        else {
            printf ("Xmodem successfully transmitted %d bytes\n", st);
        }  
        
        _closePort(portHandle);              
    }
    
    if(freeConfigFile)
        free((char *)configFileName);
    if(freeTempFile)
        free((char *)outputFileName);
        
    e4c_context_end();
}