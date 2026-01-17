EXE=y86

SRCDIR=src

# list modules without paths (just names)
MODS=p4-interp.o
OBJS=p1-check.o p2-load.o p3-disas.o
ALL_OBJS=main.o $(MODS) $(OBJS)

CC=gcc
CFLAGS=-g -O0 -Wall --std=c99 -pedantic -I$(SRCDIR)
LDFLAGS=-g -O0
LIBS=

default: $(EXE)

$(EXE): $(ALL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# compile .c files from src/ into .o files in the current directory
%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(EXE) *.o

.PHONY: default clean
