@echo off
set ARCH=%1
set TARGET=%2
if "%1"=="" set ARCH=ia32
if "%2"=="" set TARGET=all

echo *** ARCH: %ARCH%


rem CHANGE THIS PATH to TCC root
set DEVBASE=\portable\development

rem inside TCC root place 32bit TCC in tcc_ia32 directory and
rem 64bit TCC files in tcc_ia64 dirctory
set TCCBASE=%DEVBASE%\tcc\tcc_%ARCH%
set MAKE=%TCCBASE%\make.exe

%MAKE% -f Makefile.win "ARCH=%ARCH%" %TARGET%

