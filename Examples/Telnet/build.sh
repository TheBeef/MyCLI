#!/bin/sh

gcc -Wall -I ../../src -I . main.c CLI_Sockets.c ../../src/Full/CLI.c -g -o a.out
