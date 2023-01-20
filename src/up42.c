#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef linux
#include <unistd.h>
#endif

#include "e4c.h"
#include "gopt.h"
#include "ihex.h"
#include "ini.h"
#include "serial.h"
#include "tools.h"
#include "up42.h"
#include "walkera.h"
#include "xor.h"

int verbosity = 0;
int quiet = 0;
int simpleOut = 0;
const char *prgFullName;
const char *prgName;

void exitProgram(int exitCode) {
  freeAllTable();

  e4c_context_end();
  exit(exitCode);
}

void UncaughtExceptionHandler(const e4c_exception *exception) {
  printInfo(LOG_NORMAL, stderr, "Error: %s (%s)\n", exception->message,
            exception->name);
  exitProgram(EXIT_UNKNOWN_ERROR);
}

void showVersion() {
  printf("Walkera UP02 compatible firmware flasher %s (%s).\n", OS, ARCH);
  printf("Version %s, Copyright 2015, richard.prinz@min.at\n", VERSION);
}

void showUsage(char *prgName, int exitCode) {
  printf("\n");
  showVersion();
  printf("\n");
  printf("Usage: %s [-i inputFile] [-o outputFile] [-k Key] [-p Port] [-b "
         "Baud]\n\n",
         prgName);
  printf("-h,--help      Show this help screen.\n");
  printf("-V,--version   Show version information.\n");
  printf("-v,--verbose   Enable verbose mode.\n");
  printf("-c,--config    Config file. Defaults to %s.ini.\n", prgName);
  printf("-I,--info      Read board information.\n");
  printf("-P,--profile   Use model profile from config file.\n");
  printf("-q,--quiet     Be quiet. Dont output anything.\n");
  printf("-s,--simple    Use simple console output (e.g. running inside an "
         "IDE).\n");
  printf("-i,--input     Input file to send. If omitted or '-' stdin will be "
         "used.\n");
  printf("-x,--intelhex  Input file is not a binary but in INTEL hex.\n");
  printf(
      "               This option is not compatible with input stdin '-'.\n");
  printf("-o,--output    Output file after encryption with key. If '-' \n");
  printf("               stdout will be used.\n");
  printf("               If no key specified outputFile == inputFile.\n");
  printf("-k,--key       Key to XOR-encrypt inputFile with. Can be either:\n");
  printf("               * a hex string starting with '0x'\n");
  printf("               * the name of a file which content is used as key\n");
  printf("               * if not hex or file the character string itself\n");
  printf("               If omitted no encryption is performed on input.\n");
  printf("-p,--port      Name of the port to send the encrypted output file\n");
  printf("               via XModem to. If omitted nothing will be sent.\n");
  printf("-b,--baud      Baude rate to use. Defaults to 9600.\n");
  printf("-1,-2          1 or 2 stop bits.\n");
  printf("-7,-8          7 or 8 data bits.\n");
  printf("-N,-E,-O       No, Even or Odd parity.\n");
  printf("-D,--nodtr     Disable DTR control.\n");
  printf("-d,--invdtr    Invert DTR control.\n");
  printf("\n");

  if (exitCode >= 0)
    exitProgram(exitCode);
}

int main(int argc, const char **argv) {
  e4c_context_begin(E4C_FALSE);
  e4c_context_set_handlers(UncaughtExceptionHandler, NULL, NULL, NULL);

  prgFullName = (char *)*argv;
  prgName = baseName(prgFullName);
  char *w = NULL;
  char wb[MAX_STRING];
  int l = 0;
  int deleteTempOut = 0;
  int deleteHexTempOut = 0;

  const char *argument = NULL;

  const char *configFileName = NULL;
  const char *profileName = NULL;

  const char *inputFileName = NULL;
  FILE *inputFile;

  const char *outputFileName = NULL;
  FILE *outputFile;

  // const char *transferFileName = NULL;
  // FILE *transferFile;

  const char *key = NULL;
  char *parsedKey;
  int keyLen;

  const char *port = NULL;
  int baud = 9600;
  char parity = NOPARITY;
  int dataBits = 8;
  int stopBits = ONESTOPBIT;
  int noDTR = 0;
  int invDTR = 0;
  int dtrHigh = 1;
  int dtrLow = 0;
  int onlyInfo = 0;
  int inputIsHex = 0;

  setbuf(stdout, NULL);

  // configure command-line options parsing
  void *options = gopt_sort(
      &argc, argv,
      gopt_start(
          gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help")),
          gopt_option('V', 0, gopt_shorts('V'), gopt_longs("version")),
          gopt_option('q', 0, gopt_shorts('q'), gopt_longs("quiet")),
          gopt_option('s', 0, gopt_shorts('s'), gopt_longs("simple")),
          gopt_option('v', GOPT_REPEAT, gopt_shorts('v'),
                      gopt_longs("verbose")),
          gopt_option('c', GOPT_ARG, gopt_shorts('c'), gopt_longs("config")),
          gopt_option('I', 0, gopt_shorts('I'), gopt_longs("info")),
          gopt_option('P', GOPT_ARG, gopt_shorts('P'), gopt_longs("profile")),
          gopt_option('i', GOPT_ARG, gopt_shorts('i'), gopt_longs("input")),
          gopt_option('o', GOPT_ARG, gopt_shorts('o'), gopt_longs("output")),
          gopt_option('k', GOPT_ARG, gopt_shorts('k'), gopt_longs("key")),
          gopt_option('p', GOPT_ARG, gopt_shorts('p'), gopt_longs("port")),
          gopt_option('b', GOPT_ARG, gopt_shorts('b'), gopt_longs("baud")),
          gopt_option('1', 0, gopt_shorts('1'), gopt_longs("1")),
          gopt_option('2', 0, gopt_shorts('2'), gopt_longs("2")),
          gopt_option('7', 0, gopt_shorts('7'), gopt_longs("7")),
          gopt_option('8', 0, gopt_shorts('8'), gopt_longs("8")),
          gopt_option('N', 0, gopt_shorts('N'), gopt_longs("none")),
          gopt_option('E', 0, gopt_shorts('E'), gopt_longs("even")),
          gopt_option('O', 0, gopt_shorts('O'), gopt_longs("odd")),
          gopt_option('D', 0, gopt_shorts('D'), gopt_longs("nodtr")),
          gopt_option('d', 0, gopt_shorts('d'), gopt_longs("invdtr")),
          gopt_option('x', 0, gopt_shorts('x'), gopt_longs("intelhex"))));

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
  printInfo(LOG_DEBUG, stdout, "Verbosity (%d)\n", verbosity);

  // check if we should be quiet
  quiet = gopt(options, 'q');
  printInfo(LOG_DEBUG, stdout, "Quiet (%d)\n", quiet);

  // simple console output
  simpleOut = gopt(options, 's');
  printInfo(LOG_DEBUG, stdout, "Simple output (%d)\n", simpleOut);

  // DTR handling
  noDTR = gopt(options, 'D');
  printInfo(LOG_DEBUG, stdout, "No DTR handling (%d)\n", noDTR);

  if (!noDTR) {
    invDTR = gopt(options, 'd');
    printInfo(LOG_DEBUG, stdout, "Inverted DTR handling (%d)\n", invDTR);
    dtrHigh = 0;
    dtrLow = 1;
  }

  // Input file is in INTEL hex format
  inputIsHex = gopt(options, 'x');
  printInfo(LOG_DEBUG, stdout, "Input file is INTEL HEX format (%d)\n",
            inputIsHex);

  // only show board informations, dont flash
  onlyInfo = gopt(options, 'I');
  printInfo(LOG_DEBUG, stdout, "Show only board infos (%d)\n", onlyInfo);

  // get config file name
  gopt_arg(options, 'c', &configFileName);
  if (!configFileName)
    configFileName = (const char *)formatString("%s.ini", prgFullName);
  printInfo(LOG_DEBUG, stdout, "Config file (%s)\n", configFileName);
  if (!fileExists((char *)configFileName)) {
    printInfo(LOG_INFO, stdout,
              "Config file (%s) does not exist or is not readable - ignored\n",
              configFileName);
    freeTableElement((void *)configFileName);
    configFileName = NULL;
  }

  // get profile parameters
  if (gopt_arg(options, 'P', &profileName)) {
    if (!configFileName) {
      printInfo(LOG_NORMAL, stderr, "Error: -P,--profile option needs existing "
                                    "config file. Use -v for more infos.\n");
      exitProgram(EXIT_CONFIG_FILE_ERROR);
    }

    printInfo(LOG_DEBUG, stdout, "---------- Profile (%s) ----------\n",
              profileName);

    // profile input file
    if (get_private_profile_string(profileName, "input", NULL, (char *)&wb,
                                   MAX_STRING, configFileName)) {
      inputFileName = (const char *)cloneString(wb);
      printInfo(LOG_DEBUG, stdout, "Input file (%s)\n", inputFileName);
    }
    // profile output file
    if (get_private_profile_string(profileName, "output", NULL, (char *)&wb,
                                   MAX_STRING, configFileName)) {
      outputFileName = (const char *)cloneString(wb);
      printInfo(LOG_DEBUG, stdout, "Output file (%s)\n", outputFileName);
    }
    // profile Key
    if (get_private_profile_string(profileName, "key", NULL, (char *)&wb,
                                   MAX_STRING, configFileName)) {
      key = (const char *)cloneString(wb);
      printInfo(LOG_DEBUG, stdout, "Key (%s)\n", key);
    }
    // profile port
    if (get_private_profile_string(profileName, "port", NULL, (char *)&wb,
                                   MAX_STRING, configFileName)) {
      port = (const char *)cloneString(wb);
      printInfo(LOG_DEBUG, stdout, "Serial port (%s)\n", port);
    }
    // profile baud
    baud = get_private_profile_int(profileName, "baud", 0, configFileName);
    printInfo(LOG_DEBUG, stdout, "Serial baud (%d)\n", baud);
    // profile parity
    parity = get_private_profile_int(profileName, "parity", 0, configFileName);
    printInfo(LOG_DEBUG, stdout, "Serial parity (%d)\n", parity);
    // profile databits
    dataBits =
        get_private_profile_int(profileName, "dataBits", 0, configFileName);
    printInfo(LOG_DEBUG, stdout, "Serial data bits (%d)\n", dataBits);
    // profile stopbits
    stopBits =
        get_private_profile_int(profileName, "stopBits", 0, configFileName);
    printInfo(LOG_DEBUG, stdout, "Serial stop bits (%d)\n", stopBits);

    printInfo(LOG_DEBUG, stdout, "---------- End of profile ----------\n");
  }

  printInfo(LOG_DEBUG, stdout, "---------- Actual values ----------\n");

  // get serial port
  gopt_arg(options, 'p', &port);
  printInfo(LOG_DEBUG, stdout, "Serial port (%s)\n", port);

  // get input file name
  if (onlyInfo == 0) {
    gopt_arg(options, 'i', &inputFileName);
    if (inputFileName) {
      if (!strcmp(inputFileName, "-")) {
        inputFile = stdin;
        if (inputIsHex) {
          printInfo(LOG_NORMAL, stderr, "Error: INTEL HEX mode not supported "
                                        "when using stdio as input\n");
          exitProgram(EXIT_INPUT_FILE_ERROR);
        }
      } else {
        if (inputIsHex) {
          // if specified input file is in INTEL HEX form convert
          // it to binary first and use that as input
          int bl = 0;
          const char *tmpf = hex2bin(inputFileName, NULL, &bl);
          inputFileName = tmpf;
          deleteHexTempOut = 1;
        }

        inputFile = fopen(inputFileName, "rb");
      }

      if (!inputFile) {
        printError(stderr, "opening input file (%s)", inputFileName);
        exitProgram(EXIT_INPUT_FILE_ERROR);
      }
    } else {
      printInfo(LOG_NORMAL, stderr, "Error: No input file specified!\n");
      exitProgram(EXIT_INPUT_FILE_ERROR);
    }
  } else
    inputFileName = NULL;
  printInfo(LOG_DEBUG, stdout, "Input file (%s)\n", inputFileName);

  // get output file name
  if (onlyInfo == 0) {
    gopt_arg(options, 'o', &outputFileName);
    if (!outputFileName) {
      // create temp file as output file
      outputFileName = getTempFile((char *)prgName);
      deleteTempOut = 1;
    }

    if (outputFileName) {
      if (!strcmp(outputFileName, "-")) {
        if (port) {
          printInfo(LOG_NORMAL, stderr, "Error: stdout as output file can only "
                                        "be specified without port.\n");
          exitProgram(EXIT_OUTPUT_FILE_ERROR);
        }
        outputFile = stdout;
      } else
        outputFile = fopen(outputFileName, "wb");

      if (!outputFile) {
        printError(stderr, "opening output file (%s)", outputFileName);
        exitProgram(EXIT_OUTPUT_FILE_ERROR);
      }
    }
  } else
    outputFileName = NULL;
  printInfo(LOG_DEBUG, stdout, "Output file (%s)\n", outputFileName);

  // get key
  if (onlyInfo == 0) {
    gopt_arg(options, 'k', &key);
    if (key) {
      printInfo(LOG_DEBUG, stdout, "Key: %s\n", key);
      parsedKey = parseKey((char *)key, &keyLen);
      printInfo(LOG_INFO, stdout,
                "Encrypt input file with key (hex dump follows):\n");
      showDump(LOG_INFO, stdout, parsedKey, keyLen);
    } else {
      printInfo(LOG_DEBUG, stdout, "No key\n");
      // create dummy key which does nothing
      parsedKey = "\0";
      keyLen = 1;
    }
  } else
    key = NULL;

  // serial baud
  if (gopt_arg(options, 'b', &argument)) {
    baud = (int)strtol(argument, &w, 0);
    if (w == argument) {
      printInfo(LOG_NORMAL, stderr, "Error: (%s) invalid baud rate\n",
                argument);
      exitProgram(EXIT_BAUD_ERROR);
    }
  }
  if (baud <= 0)
    baud = 9600;
  printInfo(LOG_DEBUG, stdout, "Serial baud (%d)\n", baud);

  // serial parity
  parity = (gopt(options, 'N') ? NOPARITY : parity);
  parity = (gopt(options, 'E') ? EVENPARITY : parity);
  parity = (gopt(options, 'O') ? ODDPARITY : parity);
  printInfo(LOG_DEBUG, stdout, "Serial parity (%d)\n", parity);

  // serial data bits
  dataBits = (dataBits > 0 ? dataBits : 8);
  dataBits = (gopt(options, '7') ? 7 : dataBits);
  dataBits = (gopt(options, '8') ? 8 : dataBits);
  printInfo(LOG_DEBUG, stdout, "Serial data bits (%d)\n", dataBits);

  // serial stop bits
  stopBits = (gopt(options, '1') ? ONESTOPBIT : stopBits);
  stopBits = (gopt(options, '2') ? TWOSTOPBITS : stopBits);
  printInfo(LOG_DEBUG, stdout, "Serial stop bits (%d)\n", stopBits);

  printInfo(LOG_DEBUG, stdout, "---------- End of actual values ----------\n");

  // done with processing command line arguments
  gopt_free(options);

  // -----------------------------------------------------------------------------

  if (onlyInfo == 0) {
    try {
      xorFile(inputFile, outputFile, parsedKey, keyLen);
    } catch (RuntimeException) {
      const e4c_exception *exception = e4c_get_exception();
      e4c_print_exception(exception);
      exitProgram(EXIT_UNKNOWN_ERROR);
    } finally {
      if (inputFile != stdin)
        fclose(inputFile);

      if (outputFile != stdout)
        fclose(outputFile);
    }
  }

  if (port) {
    // open serial port
    HANDLE portHandle = serial_openPort(port, baud, parity, dataBits, stopBits);
    if (portHandle == INVALID_HANDLE_VALUE) {
      printError(stderr, "opening port (%s)", port);
      exitProgram(EXIT_COMM_ERROR);
    }

    if (!noDTR) {
      printInfo(LOG_DEBUG, stdout, "Set port DTR low (%d)\n", dtrLow);
      if (serial_setDTR(portHandle, dtrLow))
        printInfo(LOG_NORMAL, stderr, "\nError: Unable to set DTR on port(%s)\n",
                  port);
    }

    printInfo(LOG_DEBUG, stdout, "Serial port (%s) opend (%d,%c,%d,%s)\n", port,
              baud, (parity == NOPARITY
                         ? 'N'
                         : (parity == ODDPARITY
                                ? 'O'
                                : (parity == EVENPARITY
                                       ? 'E'
                                       : (parity == MARKPARITY
                                              ? 'M'
                                              : (parity == 'S' ? 'S' : '-'))))),
              dataBits,
              (stopBits == 0
                   ? "1"
                   : (stopBits == 1 ? "1.5" : (stopBits == 2 ? "2" : "-"))));

    // connect to board
    printInfo(LOG_NORMAL, stdout, "Connecting ...%c",
      (verbosity >= LOG_DEBUG ? '\n' : '\0'));

    // reset board via serial DTR low
    if (!noDTR) {
      printInfo(LOG_DEBUG, stdout, "Set port DTR high (%d)\n", dtrHigh);
      if (serial_setDTR(portHandle, dtrHigh))
        printInfo(LOG_NORMAL, stderr,
                  "\nError: Unable to clear DTR on port (%s)\n", port);

      delay(1000);

      printInfo(LOG_DEBUG, stdout, "Set port DTR low (%d)\n", dtrLow);
      if (serial_setDTR(portHandle, dtrLow))
        printInfo(LOG_NORMAL, stderr,
                  "\nError: Unable to set DTR on port(%s)\n", port);
    }

    // connect to board bootloader
    int isConnected = connectBoard(portHandle, 10000, 1);
    if (verbosity < LOG_COMM)
      printInfo(LOG_NORMAL, stdout, "\n");

    if (!isConnected) {
      printInfo(LOG_NORMAL, stderr, "Error: unable to connect on port (%s)\n",
                port);
      exitProgram(EXIT_COMM_ERROR);
    }

#ifdef linux
    serial_resetPort(portHandle);
#endif

    // query board info
    if (getBoardInfo(portHandle, 10000, (char *)&wb, MAX_STRING) < 0)
      printInfo(LOG_NORMAL, stderr,
                "Warning: unable to identify receiver board\n");
    else
      printInfo(LOG_NORMAL, stdout, "Connected to: %s\n", wb);

    // Flash firmware only if not requesting board infos
    if (onlyInfo == 0) {
      printInfo(LOG_NORMAL, stdout, "Flashing ...\n");

      // flash firmware
      l = flashBoard(portHandle, 10000, outputFileName);
      printInfo(LOG_NORMAL, stdout, "\n");

      if (l < 0)
        printInfo(LOG_NORMAL, stderr, "Warning: unable to flash board (%d)\n",
                  l);
      else
        printInfo(LOG_NORMAL, stdout, "Flashed %d bytes\n", l);
    }

    // close port
    disconnectBoard(portHandle);
    serial_closePort(portHandle);
  }

  // delete temporary files
  if (deleteTempOut) {
    printInfo(LOG_DEBUG, stdout, "Delete temporary file (%s)\n",
              outputFileName);
    unlink(outputFileName);
  }
  if (deleteHexTempOut) {
    printInfo(LOG_DEBUG, stdout, "Delete temporary file (%s)\n", inputFileName);
    unlink(inputFileName);
  }

  exitProgram(EXIT_OK);
  return EXIT_OK;
}
