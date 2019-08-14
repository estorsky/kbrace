CC = gcc
FLAGS = -pedantic -Wall -Wextra -O2 # -Werror
CFLAGS = -g
LIBS = -lpthread $(shell ncursesw5-config --libs)
SRCDIR = src
LIBDIR = lib
OBJDIR = obj
SRC = $(wildcard $(SRCDIR)/*.c)
LIB = $(wildcard $(LIBDIR)/*.c)
OBJ =  $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
OBJ += $(patsubst $(LIBDIR)/%.c, $(OBJDIR)/%.o, $(LIB))
EXE = ../kbrace_client

.PHONY: all clean

all: mkdir $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

$(OBJDIR)/%.o: $(LIBDIR)/%.c
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

mkdir:
	mkdir -p $(OBJDIR)

clean:
	# rm -rf $(OBJ)
	rm -rf $(OBJDIR)
	rm -rf $(EXE)

