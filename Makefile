#
# Makefile for simplekv example
#

PROGS = warmup simplekv_simple simplekv_word_count find_bugs queue queue_pmemobj queue_pmemobj_cpp
CXXFLAGS = -g -std=c++11 -DLIBPMEMOBJ_CPP_VG_PMEMCHECK_ENABLED=1 `pkg-config --cflags valgrind`
LIBS = -lpmemobj

all: $(PROGS)

warmup: warmup.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

simplekv_simple: simplekv_simple.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

simplekv_word_count: simplekv_word_count.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

find_bugs: find_bugs.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

queue: queue.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

queue_pmemobj: queue_pmemobj.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

queue_pmemobj_cpp: queue_pmemobj_cpp.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

clean:
	$(RM) *.o

clobber: clean
	$(RM) $(PROGS)

.PHONY: all clean clobber
