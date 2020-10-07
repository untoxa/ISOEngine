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
@set OPTIMIZE=0

@set CFLAGS=-mgbz80 --fsigned-char --no-std-crt0 -I%GBDK%include -I%GBDK%include\asm -I%SRC%include -I%OBJ%. -c
@set CFLAGS=%CFLAGS% --max-allocs-per-node 50000

@set LNAMES=-g _shadow_OAM=0xC000 -g .OAM=0xC000 -g .STACK=0xE000 -g .refresh_OAM=0xFF80 -b _DATA=0xc0a0 -b _CODE=0x0200
@set LFLAGS=-n -j -i -k %GBDKLIB%\gbz80\ -l gbz80.lib -k %GBDKLIB%\gb\ -l gb.lib %LNAMES%
@set LFILES=%GBDKLIB%gb\crt0.o

@set ASMFLAGS=-plosgff -I%GBDKLIB%

@set BINFLAGS=-yt 0x1a -yo 4 -ya 4

@if %1. == debug. (
    @echo Debugging mode ON
    @set DEBUGGING=1
)
 
@if %1. == profile. (
    @set PROFILING=1 
    @echo Profilig mode ON
)

@if %1. == optimize. (
    @set OPTIMIZE=1 
    @echo Optimization rules ON
)

@if %OPTIMIZE%==1 @set CFLAGS=%CFLAGS% --peep-file peephole\gbz80.rul --fverbose-asm
@if %PROFILING%==1 @set CFLAGS=%CFLAGS% --profile -DPROFILING
@if %DEBUGGING%==1 (
    @set CFLAGS=%CFLAGS% --debug -DDEBUGGING --nolospre
    @set LFLAGS=%LFLAGS% -y -m -w
)

@echo Cleanup...

@if exist %OBJ% rd /s/q %OBJ%
@if exist %PROJ%.gb del %PROJ%.gb
@if exist %PROJ%.sym del %PROJ%.sym
@if exist %PROJ%.map del %PROJ%.map
@if exist %PROJ%.noi del %PROJ%.noi
@if exist %PROJ%.cdb del %PROJ%.cdb
@if exist %PROJ%.ihx del %PROJ%.ihx

@if not exist %OBJ% mkdir %OBJ%

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
        compat.s
        MBC1_RAM_INIT.s
       ) do call :doassemble %SRC% %%x

@for %%x in (
	nonintrinsic.c
        shadow.c
        mapping.c
        clipping.c
        scenes.c
        multiple.c
        effects.c
        misc_resources.c
	transform.c
        %PROJ%.c
       ) do call :docompile %SRC% %%x

@echo LINKING...
sdldgb %LFLAGS% %PROJ%.ihx %LFILES%
IF %ERRORLEVEL% NEQ 0 goto :error

@echo MAKING BIN...
makebin -Z %BINFLAGS% %PROJ%.ihx %PROJ%.gb

@echo DONE!
@goto :eof

:docompile
@echo %2
sdcc %CFLAGS% %1%2 -o %OBJ%%2.rel
IF %ERRORLEVEL% NEQ 0 goto :error
@set LFILES=%LFILES% %OBJ%%2.rel
goto :eof

:doassemble
@echo %2
sdasgb %ASMFLAGS% %OBJ%%2.rel %1%2
IF %ERRORLEVEL% NEQ 0 goto :error
@set LFILES=%LFILES% %OBJ%%2.rel
goto :eof

:error
@echo ERROR !
exit 0
