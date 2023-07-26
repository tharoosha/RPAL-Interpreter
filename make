# Makefile for rpal_final

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17

# select the operating system
ifeq ($(OS),Windows_NT)
	RM = del /Q
	RM_CLEAN = del /Q *.o rpal20.exe
else
	RM_CLEAN = rm -f *.o rpal20

	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		RM = rm -f
	endif
	ifeq ($(UNAME_S),Darwin)
		RM = rm -f
	endif
endif

# Source files and object files
SRCS :=  asttost.cpp cse.cpp flattenst.cpp lexicon.cpp psg.cpp rpal-interpreter.cpp
OBJS := $(SRCS:.cpp=.o)

# Header files
HDRS :=  asttost.hpp cse.hpp flattenst.hpp lexicon.hpp psg.hpp 

# Target executable
TARGET := rpal20

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	$(RM) *.o

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Header dependencies
$(OBJS): $(HDRS)

# Clean
clean:
	$(RM_CLEAN)
