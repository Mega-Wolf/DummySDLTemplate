@echo off

set CompilerFlags= ^
 -MTd ^
 -I"include/SDL" ^
 -nologo ^
 -W4 -wd4201 -wd4505 ^
 -Oi -GR- -EHa- -Gm- ^
 -FC -Zi

set LinkerFlags= -opt:ref libs\* opengl32.lib

cl %CompilerFlags% .\code\main.cpp /Fe:main.exe /link %LinkerFlags% 

REM -MT -subsystem:windows,5.1

REM Z7 instead of Zi
REM -Oi           use intrinsics
REM -GR-          turn of type info
REM -EHa-         turn of exception handling
REM -Gm-          turn of incremental build (actually just stuff for it)
REM -opt:ref      removes all functions that are not referenced


if ERRORLEVEL 1 (
      echo [31m
      echo ########################### ERROR ###########################
      echo ########################### ERROR ###########################
      echo ########################### ERROR ###########################
      echo [0m

      @echo Completed: [31m%date% %time%[0m
) else (
      echo [32m
      echo ########################### SUCCESS ###########################
      echo ########################### SUCCESS ###########################
      echo ########################### SUCCESS ###########################
      echo [0m

      @echo Completed: [32m%date% %time%[0m
)
