@echo off

if not exist build mkdir build
cl /nologo /std:c11 /MT /W4 /D_CRT_SECURE_NO_WARNINGS /Febuild\bbs.exe src\bbs.c
