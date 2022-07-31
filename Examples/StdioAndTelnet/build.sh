#!/bin/sh

gcc -Wall -I ../../src -I . main.c CLI_Sockets.c CLI_Stdio.c ../../src/Full/CLI.c -g -o a.out
