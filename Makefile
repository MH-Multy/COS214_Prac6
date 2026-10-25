CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++20 -g
LDFLAGS  := 

TARGET   := main

SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(TARGET)

docs:
	doxygen Doxyfile

clean:
	rm -f $(OBJS) $(TARGET)

clean-docs:
	rm -rf docs/html docs/latex

.PHONY: all run valgrind docs clean clean-docs