TARGET = modulo

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -Wno-unused-parameter
LFLAGS = -lcjson -lncurses
DEBUG_FLAGS = -g

# Dirs
SRCDIR  := $(shell find ./src -type d)
INCDIR  := $(SRCDIR)
BINDIR  := ./bin
DATADIR := $(HOME)/.config/modulo

PREFIX = /usr/local

# src files
SRC := $(wildcard $(addsuffix /*.c, $(SRCDIR)))
INC := $(wildcard $(addsuffix /*.h, $(INCDIR)))

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

.PHONY: install
install: $(BINDIR)/$(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(BINDIR)/$(TARGET) $(DESTDIR)$(PREFIX)/bin/

$(BINDIR)/$(TARGET): $(SRC) $(INC)
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) $(SRC) -o $@ $(LFLAGS)