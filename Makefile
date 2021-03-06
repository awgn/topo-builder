CXX=g++-4.6

CPPFLAGS=-g -std=c++0x -O3 -DNDEBUG -Wall -Wextra -I . -I lib ${EXTRA_FLAGS} 
LDFLAGS=-g 
LDLIBS=-pthread

SRCS= topo-builder.cpp builder.cpp script.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: topo-builder

topo-builder: $(OBJS)
	    $(CXX) $(LDFLAGS) -o topo-builder $(OBJS) $(LDLIBS) $(INCLUDES) 

clean:
	    $(RM) $(OBJS) topo-builder 

dist-clean: clean
	    $(RM) *~ *.o

