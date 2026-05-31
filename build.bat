@echo off

if not exist build mkdir build
clang -std=gnu11 -Wall -D_CRT_SECURE_NO_WARNINGS -Wpedantic -Wno-unused-function src\bbs.c -o build\bbs.exe