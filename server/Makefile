CC = gcc
FLAGS = -pedantic -Wall -Wextra -O2 # -Werror
CFLAGS = -g
LIBS = -lpthread #$(shell ncursesw5-config --libs) # -static
SRCDIR = src
LIBDIR = lib
OBJDIR = obj
INCDIR = include
SHINCDIR = ../shared/include
SRC = $(wildcard $(SRCDIR)/*.c)
LIB = $(wildcard $(LIBDIR)/*.c)
INC = $(wildcard $(INCDIR)/*.h)
INC += $(wildcard $(SHINCDIR)/*.h)
OBJ =  $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
OBJ += $(patsubst $(LIBDIR)/%.c, $(OBJDIR)/%.o, $(LIB))
EXE = ../kbrace_server

.PHONY: all clean

all: mkdir $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

$(OBJDIR)/%.o: $(LIBDIR)/%.c $(INC)
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC)
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

mkdir:
	mkdir -p $(OBJDIR)

clean:
	# rm -rf $(OBJ)
	rm -rf $(OBJDIR)
	rm -rf $(EXE)

