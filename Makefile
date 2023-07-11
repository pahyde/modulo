TARGET = main

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -Wno-unused-parameter

# Dirs
BINDIR = ./bin
SRCDIR = ./src
INCDIR = $(SRCDIR)

# src files
SRC := $(wildcard $(SRCDIR)/*.c)
INC := $(wildcard $(INCDIR)/*.h)

$(BINDIR)/$(TARGET): $(SRC) $(INC)
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) $(SRC) -o $(BINDIR)/$(TARGET)

.PHONY: run
run:
	@$(BINDIR)/$(TARGET)

.PHONY: clean
clean:
	@rm -rf $(BINDIR)

.PHONY: all
all: $(BINDIR)/$(TARGET)
	@$(MAKE) -s run