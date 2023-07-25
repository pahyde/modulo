TARGET = main

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -Wno-unused-parameter
LFLAGS = -lcjson -lncurses
DEBUG_FLAGS = -g

# Dirs
SRCDIR  = ./src
INCDIR  = $(SRCDIR)
BINDIR  = ./bin
DATADIR = $(HOME)/.config/modulo

# src files
SRC := $(wildcard $(SRCDIR)/*.c)
INC := $(wildcard $(INCDIR)/*.h)

.PHONY: dev
dev: $(BINDIR)/$(TARGET)

.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(BINDIR)/$(TARGET)

.PHONY: clean
clean:
	@rm -rf $(BINDIR)

.PHONY: init
init:
	@rm -rf $(BINDIR)
	@rm -rf $(DATADIR)

.PHONY: all
all: $(BINDIR)/$(TARGET)
	@$(MAKE) -s run

$(BINDIR)/$(TARGET): $(SRC) $(INC)
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) $(SRC) -o $@ $(LFLAGS)