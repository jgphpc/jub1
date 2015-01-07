include ../Makefile.common

SRCS     = $(wildcard *.cc)
OBJS     = $(SRCS:.cc=.o)


.PHONY: all clean


all: $(OBJS)


.cc.o:
#	echo "Compiling $@: $(CXX) $(CFLAGS) $< -o $@"
	$(CXX) $(CFLAGS) $< -o $@


clean:
	rm -f *.o *.so.* *.a *~


force_look:
	true



# Dependencies
