CXX=g++

CPPFLAGS=-g -std=c++11 -O3 -DNDEBUG -Wall -Wextra -pedantic -I . -I lib   
LDFLAGS=-g 
LDLIBS=-pthread

SRCS= main.cpp builder.cpp builder-script.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: topo-builder

topo-builder: $(OBJS)
	    $(CXX) $(LDFLAGS) -o topo-builder $(OBJS) $(LDLIBS) $(INCLUDES) 

clean:
	    $(RM) $(OBJS) topo-builder 

dist-clean: clean
	    $(RM) *~ 

