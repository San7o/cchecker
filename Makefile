include tests/make.config

TESTS_SOURCES?=
CXX?=gcc
CXXFLAGS:=$(CXXFLAGS) -Wall -Werror -Wconversion -Wpedantic -ggdb
OBJS= $(TESTS_SOURCES:.cpp=.o)
DEPS= $(TESTS_SOURCES:.cpp=.d)

.PHONY: clean

all: check
check: # Build and run tests
check: test
	@chmod +x test && ./test
	@echo -e "Tests completed"

test: # build test target
test: $(OBJS)
	@echo " * [LD] test"
	@$(CXX) $(OBJS) -o test $(CXXFLAGS)

%.o: %.cpp
	@echo " * [CXX] $@"
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

%.d: %.cpp
	@echo " * [CXX] $@"
	@$(CXX) -MM -MF $@ -MT $(@:.d=.o) $<

-include $(DEPS)

clean: # Clean the build files
	@echo " * Cleaning build"
	@rm test $(DEPS) $(OBJS) || :
