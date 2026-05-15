UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE
    LIBVMAPP=libvm_app_macos.o
    LIBVMPAGER=libvm_pager_macos.o
else
    CC=g++
    LIBVMAPP=libvm_app.o
    LIBVMPAGER=libvm_pager.o
endif

CXXFLAGS = -g -Wall -fno-builtin -std=c++17

# List of source files for your pager
PAGER_SOURCES=vm_pager.cpp utils.cpp

# Generate the names of the pager's object files
PAGER_OBJS=${PAGER_SOURCES:.cpp=.o}

# List of all test programs
TEST_SOURCES=$(wildcard test_*.cpp)
TEST_PROGRAMS=$(TEST_SOURCES:.cpp=)

all: pager

test: pager $(TEST_PROGRAMS)

# Compile the pager and tag this compilation
pager: ${PAGER_OBJS} ${LIBVMPAGER}
	./autotag.sh push
	${CC} $(CXXFLAGS) -o $@ $^

# Compile an application program
test_%: test_%.cpp ${LIBVMAPP}
	${CC} ${CXXFLAGS} -o $@ $^ -ldl

all_tests: $(patsubst %.cpp, %, $(wildcard test_*.cpp))

run_tests: all_tests
	@for test in $(TEST_PROGRAMS); do \
		echo "Running $$test..."; \
		./$$test; \
	done

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} $(CXXFLAGS) -c $<
%.o: %.cc
	${CC} $(CXXFLAGS) -c $<

clean:
	rm -f ${PAGER_OBJS} pager $(TEST_PROGRAMS)
