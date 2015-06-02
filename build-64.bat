@echo off
echo *** Intel IA64 bit target

set DEVBASE=\portable\development
set TCCBASE=%DEVBASE%\tcc_ia64
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
echo e4c.c
%CC% -c .\src\e4c.c -o .\bin\e4c.o
echo tools.c
%CC% -c .\src\tools.c -o .\bin\tools.o
echo ini.c
%CC% -c .\src\ini.c -o .\bin\ini.o
echo gopt.c
%CC% -c .\src\gopt.c -o .\bin\gopt.o -DDO_EXIT=exitProgram
echo serial.c
%CC% -c .\src\crc16.c -o .\bin\crc16.o
echo crc16.c
%CC% -c .\src\xmodem.c -o .\bin\xmodem.o
echo xmodem.c
%CC% -c .\src\serial.c -o .\bin\serial.o
echo xor.c
%CC% -c .\src\xor.c -o .\bin\xor.o
echo walkera.c
%CC% -c .\src\walkera.c -o .\bin\walkera.o
echo up02c.c
%CC% .\src\up02c.c ^
	.\bin\e4c.o .\bin\tools.o .\bin\ini.o .\bin\gopt.o ^
	.\bin\serial.o .\bin\crc16.o .\bin\xmodem.o .\bin\xor.o ^
	.\bin\walkera.o ^
	-o .\bin\up02c.exe
if %ERRORLEVEL% GEQ 1 EXIT /B 1

echo *** check binary
%SIG% .\bin\up02c.exe

rem del /Q *.o >nul 2>&1

rem pause
