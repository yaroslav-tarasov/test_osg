@echo off

set CURR_DIR=%~dp0
set CURR_DIR1=%CURR_DIR:~0,-1%

for %%f in (%CURR_DIR1%) do set THIS_FOLDER=%%~nxf

%SIMEX_DIR%\bin\release\conv.exe %~dp0\%THIS_FOLDER%.dae %~dp0\%THIS_FOLDER%.osgb

%SIMEX_DIR%\bin\release\daextr.exe %~dp0\%THIS_FOLDER%.dae 
