#!/bin/bash

#building from linux for windows machine

nasm -f win64 syscall.asm -o syscall.o

x86_64-w64-mingw32-gcc main.c -I. -c -o main.o

x86_64-w64-mingw32-gcc main.o syscall.o -o guduna.exe

