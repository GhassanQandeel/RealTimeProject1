# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude/
LDFLAGS = 

# OpenGL specific flags
GL_CFLAGS = 
GL_LDFLAGS = -lGL -lGLU -lglfw -lGLEW -lglut -lm

# Directories
SRCDIR = src
BINDIR = bin
OBJDIR = obj

# Source files
REF_SRC = $(SRCDIR)/referee.c $(SRCDIR)/config.c
PLAYER_SRC = $(SRCDIR)/player.c $(SRCDIR)/config.c
OPENGL_SRC = $(SRCDIR)/opengl.c

# Targets
all: $(BINDIR)/referee $(BINDIR)/player $(BINDIR)/opengl $(BINDIR)/shmfile

$(BINDIR)/referee: $(REF_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BINDIR)/player: $(PLAYER_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BINDIR)/opengl: $(OPENGL_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(GL_CFLAGS) -o $@ $< $(GL_LDFLAGS)

$(BINDIR)/shmfile:
	@mkdir -p $(BINDIR)
	@touch $@

run: all
	@rm -f /tmp/fifo_*
	./$(BINDIR)/referee config.txt

run-opengl: $(BINDIR)/opengl
	./$(BINDIR)/opengl

clean:
	rm -rf $(BINDIR)/*
	rm -f /tmp/fifo_*

.PHONY: all clean run run-opengl