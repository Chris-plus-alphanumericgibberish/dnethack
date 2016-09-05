@REM  SCCS Id: @(#)nhsetup.bat  3.4     $Date: 2002/07/24 08:25:21 $
@REM  Copyright (c) NetHack PC Development Team 1993, 1996, 2002
@REM  NetHack may be freely redistributed.  See license for details. 
@REM  Win32 setup batch file, see Install.nt for details
@REM
@echo off

set _pause=

:nxtcheck
echo Checking to see if directories are set up properly
if not exist ..\..\include\hack.h goto :err_dir
if not exist ..\..\src\hack.c goto :err_dir
if not exist ..\..\dat\wizard.des goto :err_dir
if not exist ..\..\util\makedefs.c goto :err_dir
if not exist ..\..\sys\winnt\winnt.c goto :err_dir
echo Directories look ok.


:do_win
if not exist ..\..\win\win10\nethack.sln goto :err_win
echo.
echo Hard Linking Visual C project files to ..\..\build directory
echo Hard Linking ..\..\win\win10\nethack.sln to ..\..\nethack.sln
mklink /H ..\..\nethack.sln ..\..\win\win10\nethack.sln >nul
if NOT exist ..\..\binary\*.* echo Creating ..\..\binary directory
if NOT exist ..\..\binary\*.* mkdir ..\..\binary
if NOT exist ..\..\binary\license copy ..\..\dat\license ..\..\binary\license >nul
if NOT exist ..\..\build\*.* echo Creating ..\..\build directory
if NOT exist ..\..\build\*.* mkdir ..\..\build
for %%G in (..\..\win\win10\*.vcxproj ..\..\win\win10\*.vcxproj.filters ..\..\win\win10\*.mak) Do mklink /H ..\..\build\%%~nxG %%G >nul

goto :done

:err_win
echo Some of the files needed to build graphical NetHack
echo for Windows are not in the expected places.
echo Check "Install.10" for a list of the steps required 
echo to build NetHack.
goto :fini

:err_data
echo A required file ..\..\dat\data.bas seems to be missing.
echo Check "Files." in the root directory for your NetHack distribution
echo and make sure that all required files exist.
goto :fini

:err_dir
echo Your directories are not set up properly, please re-read the
echo documentation and sys/winnt/Install.10.
goto :fini

:done
echo done!
echo.
echo Proceed with the next step documented in Install.nt 
echo.

:fini
:end
set _pause=Y
if "%0"=="nhsetup" set _pause=N
if "%0"=="NHSETUP" set _pause=N
if "%_pause%"=="Y" pause
set _pause=
