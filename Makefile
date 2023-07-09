TARGET = main

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Dirs
BINDIR = ./bin
SRCDIR = ./src

$(BINDIR)/$(TARGET): $(SRCDIR)/main.c
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) $(SRCDIR)/main.c -o $(BINDIR)/$(TARGET)

.PHONY: run
run:
	@$(BINDIR)/$(TARGET)

.PHONY: clean
clean:
	@rm -rf $(BINDIR)

.PHONY: all
all: $(BINDIR)/$(TARGET)
	@$(MAKE) -s run