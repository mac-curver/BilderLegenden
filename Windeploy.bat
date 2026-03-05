cd C:\Users\hjs\Documents\Entwicklung\Qt6\_Build\BilderLegenden\release
echo %CD%
C:\Qt\Qt6\6.10.1\mingw_64\bin\windeployqt6.exe %CD% 
@echo off
setlocal enabledelayedexpansion

set "DIR=translations"
set "KEEP=en de"

for %%f in ("%DIR%\qt_*.qm") do (
    set "FILE=%%~nxf"
    set "LANG=!FILE:~3,2!"

    set "DELETE=1"
    for %%k in (%KEEP%) do (
        if /I "!LANG!"=="%%k" set "DELETE=0"
    )

    if "!DELETE!"=="1" (
        del "%%f"
    ) else (
        echo Keep %%f
    )
)

endlocal

del qrc_*.cpp
del *.qrc
rem del *.o
pause
