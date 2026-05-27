@echo off

if not exist build mkdir build
clang -std=gnu11 -Wall -Wpedantic src\bbs.c -o build\bbs.exe
build\bbs.exe