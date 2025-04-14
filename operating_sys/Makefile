# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Source files
SRCS_FIFO = fifo.cpp
SRCS_LRU = lru.cpp

# Output executables
TARGET_FIFO = fifo
TARGET_LRU = lru

# Default build target
all: $(TARGET_FIFO) $(TARGET_LRU)

# Rule to build fifo executable
$(TARGET_FIFO): $(SRCS_FIFO)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FIFO) $(SRCS_FIFO)

# Rule to build lru executable
$(TARGET_LRU): $(SRCS_LRU)
	$(CXX) $(CXXFLAGS) -o $(TARGET_LRU) $(SRCS_LRU)

# Clean rule to remove the executables and object files
clean:
	rm -f $(TARGET_FIFO) $(TARGET_LRU)
