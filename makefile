# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude/
LDFLAGS = 

# Directories
SRCDIR = src
BINDIR = bin
OBJDIR = obj

# Source files
REF_SRC = $(SRCDIR)/referee.c $(SRCDIR)/config.c
PLAYER_SRC = $(SRCDIR)/player.c $(SRCDIR)/config.c

# Targets
all: $(BINDIR)/referee $(BINDIR)/player

$(BINDIR)/referee: $(REF_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BINDIR)/player: $(PLAYER_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run: all
	@rm -f /tmp/fifo_*
	./$(BINDIR)/referee config.txt

clean:
	rm -rf $(BINDIR)/*
	rm -f /tmp/fifo_*

.PHONY: all clean run