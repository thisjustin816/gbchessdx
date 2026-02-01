#!/bin/bash
set -e

gcc -O3 -Wall chess.c -o chess_pc

/opt/gbdk/bin/lcc -o chess.gb chess.c
