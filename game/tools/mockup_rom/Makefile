# Builds one loader ROM per screen drawn by ../castle_art.py.
#   make            all four screens into ../../build/mockup/
#   make GBDK_HOME=/opt/gbdk/
GBDK_HOME ?= /opt/gbdk/
LCC = $(GBDK_HOME)bin/lcc
PY ?= python3
OUT = ../../build/mockup
FLAGS = -Wm-yc -Wm-yC -Wm-ynKINGSIDE -I.

all: $(OUT)/castle_title.gbc $(OUT)/castle_menu.gbc $(OUT)/castle_lesson.gbc $(OUT)/castle_board.gbc

$(OUT)/castle_%.c: ../castle_art.py
	$(PY) ../castle_art.py $(OUT)

# The title runs the drifting-cloud loader; the others the plain one.
$(OUT)/castle_title.gbc: $(OUT)/castle_title.c rom_parallax.c sprites.c
	$(LCC) $(FLAGS) -o $@ rom_parallax.c sprites.c $<

$(OUT)/castle_%.gbc: $(OUT)/castle_%.c rom.c sprites.c
	$(LCC) $(FLAGS) -o $@ rom.c sprites.c $<

clean:
	rm -rf $(OUT)

.PHONY: all clean
