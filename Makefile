# ==========================================
# Settings
# ==========================================

# GBDK Paths
GBDK_ROOT = /opt/gbdk
LCC       = $(GBDK_ROOT)/bin/lcc

# Host Compiler
CC        = gcc

# Output Directories
# We separate object files to prevent architecture conflicts
OBJDIR_HOST = build/host
OBJDIR_GB   = build/gb
BINDIR      = bin

# Compilation Flags
CFLAGS_HOST = -O3 -Wall
# LCC flags: -Wm-yn="NAME" names the ROM, -Wl-j prints the map
CFLAGS_GB   = 

# Helper: Find all source files for formatting
ALL_SRC = $(wildcard *.c *.h)

# ==========================================
# Targets & Dependencies
# ==========================================

# The shared logic files
COMMON_SRC = chess.c eval.c

# Define the binaries here

# Perft
PERFT_SRC     = perft.c
PERFT_OBJ_HOST = $(addprefix $(OBJDIR_HOST)/, $(COMMON_SRC:.c=.o) $(PERFT_SRC:.c=.o))
PERFT_OBJ_GB   = $(addprefix $(OBJDIR_GB)/,   $(COMMON_SRC:.c=.o) $(PERFT_SRC:.c=.o))

# Random Game
RND_SRC       = random_game.c
RND_OBJ_HOST   = $(addprefix $(OBJDIR_HOST)/, $(COMMON_SRC:.c=.o) $(RND_SRC:.c=.o))
RND_OBJ_GB     = $(addprefix $(OBJDIR_GB)/,   $(COMMON_SRC:.c=.o) $(RND_SRC:.c=.o))

# Oneply Game
ONEPLY_SRC       = one_ply.c
ONEPLY_OBJ_HOST   = $(addprefix $(OBJDIR_HOST)/, $(COMMON_SRC:.c=.o) $(ONEPLY_SRC:.c=.o))
ONEPLY_OBJ_GB     = $(addprefix $(OBJDIR_GB)/,   $(COMMON_SRC:.c=.o) $(ONEPLY_SRC:.c=.o))

# Main Rules
all: host gb

# Group targets
host: $(BINDIR)/perft $(BINDIR)/random_game $(BINDIR)/one_ply
gb:   $(BINDIR)/perft.gb $(BINDIR)/random_game.gb $(BINDIR)/one_ply.gb

# ==========================================
# Linking Rules
# ==========================================

# --- Host Linking ---

$(BINDIR)/perft: $(PERFT_OBJ_HOST)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_HOST) -o $@ $^

$(BINDIR)/random_game: $(RND_OBJ_HOST)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_HOST) -o $@ $^

$(BINDIR)/one_ply: $(ONEPLY_OBJ_HOST)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_HOST) -o $@ $^

# --- Game Boy Linking ---

$(BINDIR)/perft.gb: $(PERFT_OBJ_GB)
	@mkdir -p $(@D)
	$(LCC) $(CFLAGS_GB) -o $@ $^

$(BINDIR)/random_game.gb: $(RND_OBJ_GB)
	@mkdir -p $(@D)
	$(LCC) $(CFLAGS_GB) -o $@ $^

$(BINDIR)/one_ply.gb: $(ONEPLY_OBJ_GB)
	@mkdir -p $(@D)
	$(LCC) $(CFLAGS_GB) -o $@ $^

# ==========================================
# Compilation Rules
# ==========================================

$(OBJDIR_HOST)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_HOST) -c -o $@ $<

$(OBJDIR_GB)/%.o: %.c
	@mkdir -p $(@D)
	$(LCC) $(CFLAGS_GB) -c -o $@ $<

# ==========================================
# Utilities
# ==========================================

# Format Target
# -------------------------
fmt:
	@echo "Formatting all .c and .h files..."
	@for f in $(ALL_SRC); do \
		echo "Processing $$f..."; \
		sed -E -i 's/(\/\/.*)[0-9]+\.\s*(.*)/\1\2/' "$$f"; \
		indent -kr -brf -nut -l0 "$$f"; \
	done
	@echo "Formatting complete."

# Clean Target
# -------------------------
clean:
	rm -rf build bin *~ *.map *.noi *.ihx

.PHONY: all host gb fmt clean
