@echo off

if not exist build mkdir build
gcc -std=gnu11 -O3 -DNDEBUG -static -static-libgcc -Wall -D_CRT_SECURE_NO_WARNINGS -Wpedantic -Wno-unused-function src\bbs.c -o build\bbs.exe
