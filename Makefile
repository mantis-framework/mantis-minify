objects=mantis-minify.o
cppfiles=mantis-minify.cc

CXX?=g++
CXXFLAGS+=-std=c++11 -Wall -Wextra -pedantic -O3

ifeq ($(small_exec), true)
	CXXFLAGS+=-Os -fno-unroll-loops -s -z noseparate-code
endif
#CXXFLAGS+=-Os -fno-unroll-loops -s -z noseparate-code

###

all: mantis-minify

###

mantis-minify: $(objects)
	$(CXX) $(CXXFLAGS) $(objects) -o mantis-minify -pthread

mantis-minify.o: mantis-minify.cc mantis-minify.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

###

mantis-minify.js: mantis-minify.emscripten.cc mantis-minify.h
	em++ -O3 -lnodefs.js mantis-minify.emscripten.cc -o mantis-minify.js

###

install:
	chmod 755 mantis-minify
	sudo cp mantis-minify /usr/local/bin/

clean:
	rm -f $(objects)

clean-all: clean
	rm -f mantis-minify mantis-minify.js mantis-minify.wasm
