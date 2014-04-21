OBJS = r_die.o r_shiftin.o r_shiftout_file.o r_tformatter_basic.o \
    r_dump_basic.o
BINARIES = shiftin_demo formatter_demo dump_demo
HEADERS = $(wildcard r_*.h)

CXXFLAGS = -s -O2 -W -Wall
CXX = g++

.PHONY: all clean

all: libr.a $(BINARIES)

clean:
	rm -f core *.o libr.a $(BINARIES)

libr.a: $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)
	ranlib $@

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

shiftin_demo: shiftin_demo.cc libr.a $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lr

formatter_demo: formatter_demo.cc libr.a $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lr

dump_demo: dump_demo.cc libr.a $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lr
