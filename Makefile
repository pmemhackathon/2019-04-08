#
# Makefile for simplekv example
#

PROGS = simplekv-simple simplekv-word-count
CXXFLAGS = -g -Wall -Werror -std=c++11\
	-DLIBPMEMOBJ_CPP_VG_PMEMCHECK_ENABLED=1 `pkg-config --cflags valgrind`
LIBS = -lpmemobj

all: $(PROGS)

simplekv-simple: simplekv_simple.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

simplekv-word-count: simplekv_word_count.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

clean:
	$(RM) *.o

clobber: clean
	$(RM) $(PROGS)

.PHONY: all clean clobber
