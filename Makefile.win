ARCH=ia64
CC=/portable/development/tcc/tcc_$(ARCH)/tcc.exe
SC=/portable/development/devtools/sigcheck.exe
CFLAGS=-DARCH="""$(ARCH)""" -MD -Wall -c

SRCPATH=src
BINPATH=bin\$(ARCH)
SOURCES=up02c.c walkera.c xor.c serial.c xmodem.c crc16.c gopt.c ini.c tools.c e4c.c
OBJECTS=$(SOURCES:%.c=$(BINPATH)/%.o)
DEP=$(OBJECTS:.o=.d)
-include $(DEP)
EXECUTABLE=$(BINPATH)/up02c.exe

all: $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	$(SC) -q -a $@

$(BINPATH)/%.o: $(SRCPATH)/%.c 
	$(CC) $(CFLAGS) $< -o $@

clean:
	del /Q $(BINPATH)\*.o >nul 2>&1
	del /Q $(BINPATH)\*.obj >nul 2>&1
	del /Q $(BINPATH)\*.a >nul 2>&1
	del /Q $(BINPATH)\*.lib >nul 2>&1
	del /Q $(BINPATH)\*.exe >nul 2>&1
	del /Q $(BINPATH)\*.dll >nul 2>&1
	del /Q $(BINPATH)\*.exp >nul 2>&1
	del /Q $(BINPATH)\*.res >nul 2>&1

