src = transcoder.cpp
obj = transcoder.o
target = transcoder
CXX = g++
AR = ar
CXXFLAGS = -Wall -std=c++11
CPPFLAGS = 
LDFLAGS = -L./ -lini -lcrc32

$(target): libini.a libcrc32.a $(obj)
	$(CXX) $(obj) -o $(target) $(LDFLAGS)

.PHONY: clean rebuild all libini.a libcrc32.a

clean:
	rm -rf $(target) *.a *.o

rebuild: clean $(target)

all: $(target)

libini.a: libini.o
	$(AR) -rc libini.a $^

libcrc32.a: libcrc32.o
	$(AR) -rc libcrc32.a $^
