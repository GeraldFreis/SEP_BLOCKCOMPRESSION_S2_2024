# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -ICompressionFixedSize -IInput

# Source files
SRCS = \
    Test/CompressionTests/FixedSize/FixedSizeTests.cpp \
    CompressionFixedSize/CompressionFixedSize.cpp \
    Input/Input.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = mytest.out

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
