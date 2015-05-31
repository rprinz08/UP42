@echo off

set DEVBASE=\portable\development
set TCCBASE=%DEVBASE%\tcc
set TOOLBASE=%DEVBASE%\DevTools

set IC=%TCCBASE%\include
set IW=%IC%\winapi

set CC=%TCCBASE%\tcc.exe
set RC=%TOOLBASE%\rc.exe
set AR=%TCCBASE%\tiny_libmaker.exe
set DF=%TCCBASE%\tiny_impdef.exe

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
%CC% -bench -c .\src\tools.c -o .\bin\tools.o
%CC% -bench -c .\src\gopt.c -o .\bin\gopt.o
%CC% -bench -c .\src\serial.c -o .\bin\serial.o
%CC% -bench -c .\src\crc16.c -o .\bin\crc16.o
%CC% -bench -c .\src\xmodem.c -o .\bin\xmodem.o
%CC% -bench -c .\src\xor.c -o .\bin\xor.o
%CC% -bench .\src\up02c.c ^
	.\bin\tools.o .\bin\gopt.o ^
	.\bin\xor.o .\bin\crc16.o .\bin\xmodem.o .\bin\serial.o ^
	-o .\bin\up02c.exe
if %ERRORLEVEL% GEQ 1 EXIT /B 1

rem del /Q *.o >nul 2>&1

rem pause
