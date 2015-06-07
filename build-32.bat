@echo off
echo *** Intel IA32 bit target

set DEVBASE=\portable\development
set TCCBASE=%DEVBASE%\tcc_ia32
set TOOLBASE=%DEVBASE%\DevTools

set IC=%TCCBASE%\include
set IW=%IC%\winapi

set CC=%TCCBASE%\tcc.exe
set RC=%TOOLBASE%\rc.exe
set AR=%TCCBASE%\tiny_libmaker.exe
set DF=%TCCBASE%\tiny_impdef.exe
set SIG=%TOOLBASE%\sigcheck.exe

cls

echo *** change path

set P=%CD%
cd %DEVBASE%\UP02-clone

echo *** clear

del /Q .\bin\*.o >nul 2>&1
del /Q .\bin\*.obj >nul 2>&1
del /Q .\bin\*.a >nul 2>&1
del /Q .\bin\*.lib >nul 2>&1
del /Q .\bin\*.exe >nul 2>&1
del /Q .\bin\*.dll >nul 2>&1
del /Q .\bin\*.exp >nul 2>&1
del /Q .\bin\*.res >nul 2>&1

echo *** building UP02 clone
rem === static
set TARCH="x86"
echo e4c.c
%CC% -c .\src\e4c.c -o .\bin\e4c.o -DARCH=""%TARCH%""
echo tools.c
%CC% -c .\src\tools.c -o .\bin\tools.o -DARCH=""%TARCH%""
echo ini.c
%CC% -c .\src\ini.c -o .\bin\ini.o -DARCH=""%TARCH%""
echo gopt.c
%CC% -c .\src\gopt.c -o .\bin\gopt.o -DDO_EXIT=exitProgram -DARCH=""%TARCH%""
echo serial.c
%CC% -c .\src\crc16.c -o .\bin\crc16.o -DARCH=""%TARCH%""
echo crc16.c
%CC% -c .\src\xmodem.c -o .\bin\xmodem.o -DARCH=""%TARCH%""
echo xmodem.c
%CC% -c .\src\serial.c -o .\bin\serial.o -DARCH=""%TARCH%""
echo xor.c
%CC% -c .\src\xor.c -o .\bin\xor.o -DARCH=""%TARCH%""
echo walkera.c
%CC% -c .\src\walkera.c -o .\bin\walkera.o -DARCH=""%TARCH%""
echo up02c.c
%CC% .\src\up02c.c  -DARCH=""%TARCH%"" ^
	.\bin\e4c.o .\bin\tools.o .\bin\ini.o .\bin\gopt.o ^
	.\bin\serial.o .\bin\crc16.o .\bin\xmodem.o .\bin\xor.o ^
	.\bin\walkera.o ^
	-o .\bin\up02c.exe
if %ERRORLEVEL% GEQ 1 EXIT /B 1

echo *** check binary
%SIG% .\bin\up02c.exe

rem del /Q *.o >nul 2>&1

rem pause
