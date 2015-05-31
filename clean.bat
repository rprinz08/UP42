@echo off

set DEVBASE=\portable\development

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

pause
