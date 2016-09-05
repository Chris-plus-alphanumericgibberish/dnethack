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

:do_tty
@rem if NOT exist ..\..\binary\*.* mkdir ..\..\binary
@rem if NOT exist ..\..\binary\license copy ..\..\dat\license ..\..\binary\license >nul
@REM echo Copying Microsoft Makefile - Makefile.msc to ..\..\src\Makefile.
@REM if NOT exist ..\..\src\Makefile goto :domsc
@REM copy ..\..\src\Makefile ..\..\src\Makefile-orig >nul
@REM echo      Your existing
@REM echo           ..\..\src\Makefile
@REM echo      has been renamed to
@REM echo           ..\..\src\Makefile-orig
@REM :domsc
@REM copy Makefile.msc ..\..\src\Makefile >nul
@REM echo Microsoft Makefile copied ok.

@REM echo Copying Borland Makefile - Makefile.bcc to ..\..\src\Makefile.bcc
@REM if NOT exist ..\..\src\Makefile.bcc goto :dobor
@REM copy ..\..\src\Makefile.bcc ..\..\src\Makefile.bcc-orig >nul
@REM echo      Your existing 
@REM echo           ..\..\src\Makefile.bcc 
@REM echo      has been renamed to 
@REM echo           ..\..\src\Makefile.bcc-orig
@REM :dobor
@REM copy Makefile.bcc ..\..\src\Makefile.bcc >nul
@REM echo Borland Makefile copied ok.

@REM echo Copying MinGW Makefile - Makefile.gcc to ..\..\src\Makefile.gcc
@REM if NOT exist ..\..\src\Makefile.gcc goto :dogcc
@REM copy ..\..\src\Makefile.gcc ..\..\src\Makefile.gcc-orig >nul
@REM echo      Your existing
@REM echo           ..\..\src\Makefile.gcc
@REM echo      has been renamed to
@REM echo           ..\..\src\Makefile.gcc-orig
@REM :dogcc
@REM copy Makefile.gcc ..\..\src\Makefile.gcc >nul
@REM echo MinGW Makefile copied ok.

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
@REM for %%G in (..\..\win\win10\*.vcxproj.filters) Do mklink /H ..\..\build\%%~nxG %%G >nul
@REM  copy ..\..\win\win32\dgncomp.dsp   ..\..\build >nul
@REM copy ..\..\win\win32\dgnstuff.dsp  ..\..\build >nul
@REM copy ..\..\win\win32\dgnstuff.mak  ..\..\build >nul
@REM copy ..\..\win\win32\dlb_main.dsp  ..\..\build >nul
@REM copy ..\..\win\win32\levcomp.dsp   ..\..\build >nul
@REM copy ..\..\win\win32\levstuff.dsp  ..\..\build >nul
@REM copy ..\..\win\win32\levstuff.mak  ..\..\build >nul
@REM copy ..\..\win\win32\makedefs.dsp  ..\..\build >nul
@REM copy ..\..\win\win32\recover.dsp   ..\..\build >nul
@REM copy ..\..\win\win32\tile2bmp.dsp  ..\..\build >nul
@REM copy ..\..\win\win32\tiles.dsp     ..\..\build >nul
@REM copy ..\..\win\win32\tiles.mak     ..\..\build >nul
@REM copy ..\..\win\win32\tilemap.dsp   ..\..\build >nul
@REM copy ..\..\win\win32\uudecode.dsp   ..\..\build >nul
@REM copy ..\..\win\win32\nethackw.dsp   ..\..\build >nul

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
