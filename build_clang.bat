@echo off

if not exist build mkdir build
clang -std=gnu11 -O3 -DNDEBUG -fms-runtime-lib=static -Wall -D_CRT_SECURE_NO_WARNINGS -Wpedantic -Wno-unused-function src\bbs.c -o build\bbs.exe
