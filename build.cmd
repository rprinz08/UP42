@echo off
set ARCH=%1
set TARGET=%2
if "%1"=="" set ARCH=ia32
if "%2"=="" set TARGET=all

echo *** ARCH: %ARCH%

set DEVBASE=\portable\development
set TCCBASE=%DEVBASE%\tcc\tcc_%ARCH%
set MAKE=%TCCBASE%\make.exe

%MAKE% -f Makefile.win "ARCH=%ARCH%" %TARGET%

