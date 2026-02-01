#!/bin/bash
set -e

gcc -O3 -Wall chess.c perft.c -o perft
gcc -O3 -Wall chess.c random_game.c -o random_game

/opt/gbdk/bin/lcc -o perft.gb perft.c chess.c
/opt/gbdk/bin/lcc -o random_game.gb random_game.c chess.c
