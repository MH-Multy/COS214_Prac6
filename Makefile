# Compiler and flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++20 -g
LDFLAGS  := 

# Output binary name
TARGET   := main

# Find all .cpp files in current directory and map them to .o files
SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:.cpp=.o)

# Default target: builds the binary
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile each .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run with Valgrind for memory leaks and memory errors
valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(TARGET)

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Avoid filename conflicts with target names
.PHONY: all run valgrind clean