@echo off
@set PROJ=ISO
@set GBDK=..\..\gbdk\
@set GBDKLIB=%GBDK%lib\small\asxxxx\
@set OBJ=build\
@set SRC=src\
@set RES=%SRC%gfx\
@set MAP=%SRC%maps\
@set TOOLS=tools\

@set PROFILING=0
@set DEBUGGING=0
@set OPTIMIZE=1

@if %1. == debug. @set DEBUGGING=1
@if %DEBUGGING%==1 @echo Debugging mode ON
 
@if %1. == profile. @set PROFILING=1 
@if %PROFILING%==1 @echo Profilig mode ON

@set CFLAGS=-mgbz80 --no-std-crt0 -I %GBDK%include -I %GBDK%include\asm -I %SRC%include -I %OBJ% -c
@set CFLAGS=%CFLAGS% --max-allocs-per-node 50000
@if %OPTIMIZE%==1 @set CFLAGS=%CFLAGS% --peep-file peephole\gbz80.rul
@if %PROFILING%==1 @set CFLAGS=%CFLAGS% --profile -DPROFILING
@if %DEBUGGING%==1 @set CFLAGS=%CFLAGS% -DDEBUGGING

@set LFLAGS=-n -- -z -m -j -k%GBDKLIB%gbz80\ -lgbz80.lib -k%GBDKLIB%gb\ -lgb.lib 
@set LFLAGS=%LFLAGS% -yt2 -yo4 -ya4
@set LFILES=%GBDKLIB%gb\crt0.o

@set ASMFLAGS=-plosgff -I%GBDKLIB%

@echo Cleanup...

@if exist %OBJ% rd /s/q %OBJ%
@if exist %PROJ%.gb del %PROJ%.gb
@if exist %PROJ%.sym del %PROJ%.sym
@if exist %PROJ%.map del %PROJ%.map

@if not exist %OBJ% mkdir %OBJ%

@echo ASSEMBLING THE STUB...

sdasgb %ASMFLAGS% %OBJ%MBC1_RAM_INIT.rel %SRC%MBC1_RAM_INIT.s
@set LFILES=%LFILES% %OBJ%MBC1_RAM_INIT.rel

@echo COMPILING RESOURCES...

@for %%x in (
        scene_resources.b1.gbr
       ) do (
         %TOOLS%gbr2c.exe %RES%%%x %OBJ%
         call :docompile %OBJ% %%x.c
       )

@for %%x in (
        rooms.3dmap
       ) do (
         %TOOLS%mapcvt.exe %MAP%%%x %OBJ%%%x
         call :docompile %OBJ% %%x.c
       )


@echo COMPILING...

@for %%x in (
        shadow.c
        mapping.c
        clipping.c
        scenes.c
        enemies.c
        effects.c
        misc_resources.c
       ) do call :docompile %SRC% %%x

sdcc %CFLAGS% %SRC%%PROJ%.c -o %OBJ%%PROJ%.rel

@echo LINKING...
%GBDK%bin\link-gbz80 %LFLAGS% %PROJ%.gb %LFILES% %OBJ%%PROJ%.rel 

@echo DONE!
@goto :eof

:docompile
sdcc %CFLAGS% %1%2 -o %OBJ%%2.rel
@set LFILES=%LFILES% %OBJ%%2.rel
goto :eof
