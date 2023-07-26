# Makefile for rpal_final

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17

# Source files and object files
SRCS := main.cpp asttost.cpp cse.cpp flattenst.cpp lexicon.cpp psg.cpp 
OBJS := $(SRCS:.cpp=.o)

# Header files
HDRS := asttost.hpp cse.hpp flattenst.hpp lexicon.hpp psg.hpp

# Target executable
TARGET := rpal20

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	del /Q *.o

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Header dependencies
$(OBJS): $(HDRS)

# Clean
clean:
	del /Q *.o rpal20.exe
