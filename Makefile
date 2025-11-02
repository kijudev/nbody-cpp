CXX      := clang++
CXXOPT   := -O1 -g
CXXFLAGS := -std=c++23 -Wall -Wextra -Iinclude 

LIB_SRCS   := $(wildcard lib/*.cpp)
LIB_OBJS   := $(patsubst lib/%.cpp,bin/obj/%.o,$(LIB_SRCS))

APPS      := $(wildcard apps/*/main.cpp)
APP_BINS  := $(patsubst apps/%/main.cpp,bin/%,$(APPS))

TESTS     := $(wildcard tests/*.cpp)
TEST_BINS := $(patsubst tests/%.cpp,bin/tests/%,$(TESTS))

.PHONY: all apps tests run-tests clean compile-commands

all: apps tests

apps: $(APP_BINS)

tests: $(TEST_BINS)

bin/obj/%.o: lib/%.cpp
	mkdir -p bin/obj
	$(CXX) $(CXXFLAGS) $(CXXOPT) -c $< -o $@

bin/%: apps/%/main.cpp $(LIB_OBJS)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(CXXOPT) $< $(LIB_OBJS) -o $@

bin/tests/%: tests/%.cpp $(LIB_OBJS)
	mkdir -p bin/tests
	$(CXX) $(CXXFLAGS) $(CXXOPT) $< $(LIB_OBJS) -o $@

clean:
	rm -rf bin
	rm -f compile_commands.json

compile-commands: clean
	bear -- make

run-tests: tests
	@set -e; \
	for t in $(TEST_BINS); do \
	  printf "Running %s\n" "$$t"; \
	  if "$$t"; then \
	    printf "PASS$\n"; \
	  else \
	    printf "FAIL: %s\n" "$$t"; \
	    exit 1; \
	  fi \
	done
