@echo off
REM Build by Open Watcom C - C++ v1.9


if exist iolib.obj del iolib.obj
if exist mixtest.obj del mixtest.obj
if exist mixtest.exe del mixtest.exe

if "%1"=="clean" goto end

wasm -ms iolib.asm
wcl -bt=dos mixtest.c iolib.obj


:end