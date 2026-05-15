UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE -Wno-deprecated-declarations
    LIBCPU=libcpu_macos.o
else
    CC=g++ -Werror -pedantic -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG
    LIBCPU=libcpu.o
endif

CC+=-g -Wall -std=c++17

# List of source files for your thread library
THREAD_SOURCES=thread.cpp cv.cpp mutex.cpp cpu.cpp os_guard.cpp utils.cpp constants.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

all: libthread.o all_tests

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh push
	ld -r -o $@ ${THREAD_OBJS}

UNIPROCESSOR_TEST_PATH=tests/uniprocessor/
MULTIPROCESSOR_TEST_PATH=tests/multiprocessor/

# Compile an application program
test_%_u: ${UNIPROCESSOR_TEST_PATH}test_%.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_%_m: ${MULTIPROCESSOR_TEST_PATH}test_%.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Find all test files matching the patterns and build them
UNIPROCESSOR_TESTS = $(wildcard ${UNIPROCESSOR_TEST_PATH}test_*.cpp)
MULTIPROCESSOR_TESTS = $(wildcard ${MULTIPROCESSOR_TEST_PATH}test_*.cpp)

# Convert to target names without duplicating the '_u' or '_m'
UNIPROCESSOR_BINARIES = $(patsubst ${UNIPROCESSOR_TEST_PATH}test_%.cpp,test_%_u,${UNIPROCESSOR_TESTS})
MULTIPROCESSOR_BINARIES = $(patsubst ${MULTIPROCESSOR_TEST_PATH}test_%.cpp,test_%_m,${MULTIPROCESSOR_TESTS})

# Target to compile all uniprocessor and multiprocessor tests
all_tests: $(UNIPROCESSOR_BINARIES) $(MULTIPROCESSOR_BINARIES)

run_tests: all_tests
	@echo "Running uniprocessor tests..."
	@for test in $(UNIPROCESSOR_BINARIES); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

	@echo "Running multiprocessor tests..."
	@for test in $(MULTIPROCESSOR_BINARIES); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o 

clean_tests:
	rm -f test_*
