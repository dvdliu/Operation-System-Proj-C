UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE
    CXXFLAGS = -std=c++17 -I/opt/homebrew/include
    LDFLAGS += -L/opt/homebrew/lib
    LIBFSCLIENT=libfs_client_macos.o
    LIBFSSERVER=libfs_server_macos.o
    BOOST_THREAD=boost_thread-mt
else
    CC=g++
    CXXFLAGS = -std=c++17
    LDFLAGS =
    LIBFSCLIENT=libfs_client.o
    LIBFSSERVER=libfs_server.o
    BOOST_THREAD=boost_thread
endif

CC+=-g -Wall -std=c++17 -Wno-deprecated-declarations

# List of source files for your file server
FS_SOURCES=main.cpp network_server.cpp socket_raii.cpp file_system.cpp

TEST_SOURCES=$(wildcard test_*.cpp)
TEST_PROGRAMS=$(TEST_SOURCES:.cpp=)

# Generate the names of the file server's object files
FS_OBJS=${FS_SOURCES:.cpp=.o}

all: fs test

test: fs $(TEST_PROGRAMS)

# Compile the file server and tag this compilation
#
# Remember to set the CPLUS_INCLUDE_PATH, LIBRARY_PATH, and LD_LIBRARY_PATH
# environment variables to include your Boost installation directory.
fs: ${FS_OBJS} ${LIBFSSERVER}
	./autotag.sh push
	${CC} ${CXXFLAGS} -o $@ $^ -l${BOOST_THREAD} -lboost_system -pthread -ldl ${LDFLAGS}

test_%: test_%.cpp ${LIBFSCLIENT}
	${CC} ${CXXFLAGS} -o $@ $^ ${LDFLAGS}


all_tests: $(patsubst %.cpp, %, $(wildcard test_*.cpp))

run_tests: all_tests
	@for test in $(TEST_PROGRAMS); do \
		echo "Running $$test..."; \
		./$$test; \
	done

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} ${CXXFLAGS} -c $<

%.o: %.cc
	${CC} ${CXXFLAGS} -c $<


clean:
	rm -f ${FS_OBJS} fs ${TEST_PROGRAMS}
