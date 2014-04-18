OBJS = r_die.o r_shiftin.o r_shiftout_file.o r_tformatter_basic.o
CXXFLAGS = -s -O2 -W -Wall
CXX = g++

.PHONY: all

all: libr.a dump_demo formatter_demo shiftin_demo

libr.a: $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)
	ranlib $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $<

shiftin_demo: shiftin_demo.cc libr.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lr

formatter_demo: formatter_demo.cc libr.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lr

dump_demo: dump_demo.cc libr.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lr
