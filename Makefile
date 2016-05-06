# See Makefile 5 in http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
# See https://stackoverflow.com/questions/16152689/gnu-make-customize-library-search-path-in-makefile

INCDIR = .
OBJDIR = obj
LIBDIR = .
BINDIR = bin

TARGET = $(BINDIR)/cribbage.exe

# CC = gcc
CC = g++

CFLAGS = -I$(INCDIR)
# LDFLAGS = -L../lib # Can this path be relative, or must it be absolute?
LDFLAGS = -L$(LIBDIR)

# LIBS = -lm
LIBS =

_DEPS = DECK.H
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = CRIBBAGE.o DECK.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

# The importance of tabs: Please note that the commands in the bodies of rules (e.g. in this case, the line that starts with a tab and $(CC)) must start with a tab, or else "make" will reject it.
$(OBJDIR)/%.o: %.CPP $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# I really don't think that gcc needs $(CFLAGS) when it is linking rather than compiling.
# I solved the "undefined reference to `__gxx_personality_sj0'" errors by linking with g++ rather than gcc; see https://stackoverflow.com/questions/7751640/undefined-reference-to-gxx-personality-sj0
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

all: $(BINDIR)/cribbage

# How to write a "clean" rule that can be invoked via "make clean" from Powershell?
# .PHONY: clean

# In Powershell, "rm" is an alias for the Remove-Item cmdlet; that's not what we expected.  ("I didn't expect a kind of Spanish Inquisition.")
# - See https://en.wikipedia.org/wiki/The_Spanish_Inquisition_(Monty_Python)
# - See https://www.youtube.com/watch?v=Nf_Y4MbUCLY
# rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~ $(BINDIR)/* # Using the real GNU/Linux rm
# del $(OBJDIR)/*.o
# del $(BINDIR)/*
# Powershell nonsense:
# clean:
# 	Get-ChildItem -Path $(OBJDIR) -Include *.o -File | foreach { $_.Delete()}
