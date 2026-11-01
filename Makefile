CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++11 -g -I. -Itests
LDFLAGS  := -pthread

TARGET   := main

SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:.cpp=.o)

# Unit tests: every source file except main.cpp (doctest supplies its own main), plus tests/.
TEST_SRCS := $(wildcard tests/*.cpp)
TEST_OBJS := $(filter-out main.o,$(OBJS)) $(TEST_SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

demo: $(TARGET)
	./$(TARGET) --demo

run_tests: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o run_tests $(LDFLAGS)

test: run_tests
	./run_tests

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(TARGET)

docs:
	doxygen Doxyfile

clean:
	rm -f $(OBJS) $(TARGET) tests/*.o run_tests

clean-docs:
	rm -rf docs/html docs/latex

.PHONY: all run demo test valgrind docs clean clean-docs