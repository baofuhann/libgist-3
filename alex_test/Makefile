# Makefile
# Copyright (c) 2019, Fudan University

LIBDIR = $(LIBGISTHOME)/lib/
INCLUDEDIR = $(LIBGISTHOME)/include/

LIBS = $(LIBGISTHOME)/lib/libgist.a \
    $(LIBGISTHOME)/lib/libbtree.a \
    $(LIBGISTHOME)/lib/librtree.a \
    $(LIBGISTHOME)/lib/libnptree.a\
	$(LIBGISTHOME)/lib/libalex.a\


# -lstdc++ needed for the sww installation (Sparc Solaris)
CMDLINELIBS = -lnptree -lbtree -lgist -lrtree -lalex -lm -lstdc++
# CMDLINELIBS = -lnptree -lbtree -lgist -lrtree -lm -lstdc++
CFLAG = -Wcpp -g

DEST = .

# SRCS = Btree.cc
# OBJS = Btree.o

SRCS = Btree_bulk.cc
OBJS = Btree_bulk.o

# SRCS = Alex.cc
# OBJS = Alex.o

PROG = $(DEST)/BTree
# PROG = $(DEST)/Alex

$(PROG): $(LIBS)
	gcc $(SRCS) -I $(INCLUDEDIR) -L$(LIBDIR) $(CMDLINELIBS) $(CFLAG) -o $(PROG)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(PROG)
